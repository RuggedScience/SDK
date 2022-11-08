#include <iostream>

#include "rsdioimpl.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include <tinyxml2.h>

static tinyxml2::XMLError getInternalPinInfo(const tinyxml2::XMLElement *pin, int& pinId, PinConfig& info)
{
	using namespace tinyxml2;

	int id, bit, gpio;
    bool pullup = false;
	XMLError e = pin->QueryAttribute("id", &id);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("bit", &bit);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("gpio", &gpio);
	if (e != XML_SUCCESS) return e;
    e = pin->QueryAttribute("pullup", &pullup);
    if (e != XML_SUCCESS && e != XML_NO_ATTRIBUTE) return e;
	
	pinId = id;
	info = PinConfig(bit, gpio, false, pullup, false, true);
	return XML_SUCCESS;
}

static tinyxml2::XMLError getExternalPinInfo(const tinyxml2::XMLElement *pin, int& pinId, PinConfig& info)
{
	using namespace tinyxml2;

	int id, bit, gpio;
	bool invert, input, output;
	XMLError e = pin->QueryAttribute("id", &id);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("bit", &bit);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("gpio", &gpio);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("invert", &invert);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("input", &input);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("output", &output);
	if (e != XML_SUCCESS) return e;

	pinId = id;
	info = PinConfig(bit, gpio, invert, false, input, output);
	return XML_SUCCESS;
}

static tinyxml2::XMLError get8786RegData(const tinyxml2::XMLElement *reg, Ite8786::RegisterData& data)
{
    using namespace tinyxml2;

    const char* tmp = reg->Attribute("id");
    if (!tmp) return XML_NO_ATTRIBUTE;
    data.addr = std::stoi(std::string(tmp), nullptr, 0);

    tmp = reg->Attribute("ldn");
    if (tmp) data.ldn = std::stoi(std::string(tmp), nullptr, 0);
    else data.ldn = 0;

    tmp = reg->Attribute("onBits");
    if (tmp) data.onBits = std::stoi(std::string(tmp), nullptr, 0);
    else data.onBits = 0;
                
    tmp = reg->Attribute("offBits");
    if (tmp) data.offBits = std::stoi(std::string(tmp), nullptr, 0);
    else data.offBits = 0;

    return XML_SUCCESS;
}

RsDioImpl::RsDioImpl() :
    m_lastError(""),
    mp_controller(nullptr)
{}

RsDioImpl::~RsDioImpl()
{
    delete mp_controller;
}

void RsDioImpl::destroy()
{
    delete this;
}

bool RsDioImpl::setXmlFile(const char *fileName, bool debug)
{
    using namespace tinyxml2;
	m_dioMap.clear();
	if (mp_controller) delete mp_controller;
	mp_controller = nullptr;

	XMLDocument doc;
	if (doc.LoadFile(fileName) != XML_SUCCESS)
	{
        m_lastError = "XML Error: Unable to load file";
        return false;
    }

	XMLElement *comp = doc.FirstChildElement("computer");
	if (!comp)
	{
        m_lastError = "XML Error: Unable to find computer node";
        return false;
    }

	XMLElement *dio = comp->FirstChildElement("dio_controller");
	if (!dio)
	{
        m_lastError = "XML Error: Unable to find dio_controller node";
        return false;
    }

	std::string id(dio->Attribute("id"));
    try
    {
        if (debug)
        {
            std::cout << "XML DIO Controller ID: " << id << std::endl;
        }
            
	    if (id == "ite8783")
        {
    		mp_controller = new Ite8783(debug);
        }
    	else if (id == "ite8786")
        {
            Ite8786::RegisterList_t list;
            XMLElement *reg = dio->FirstChildElement("register");
            for (; reg; reg = reg->NextSiblingElement("register"))
            {
                Ite8786::RegisterData data;
                if (get8786RegData(reg, data) == XML_SUCCESS)
                    list.emplace_back(data);
            }
		    mp_controller = new Ite8786(list, debug);
        }
	    else
	    {
    		m_lastError = "XML Error: Invalid id found for dio_controller";
		    return false;
	    }
    }
    catch (std::exception &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return false;
    }

    // Print the registers before we initialize all the pins.
    if (debug)
    {
        mp_controller->printRegs();
    }

	XMLElement *con = dio->FirstChildElement("connector");
	if (!con)
	{
        m_lastError = "XML Error: Unable to find connector node";
        return false;
    }

	for (; con; con = con->NextSiblingElement("connector"))
	{
		int conId = 0;
		if (con->QueryAttribute("id", &conId) == XML_SUCCESS)
		{
			XMLElement *ip = con->FirstChildElement("internal_pin");
			for (; ip; ip = ip->NextSiblingElement("internal_pin"))
			{
				int pinId;
				PinConfig info;
				if (getInternalPinInfo(ip, pinId, info) == XML_SUCCESS)
				{
					mp_controller->initPin(info);
					m_dioMap[conId][pinId] = info;
				}
			}

			XMLElement *ep = con->FirstChildElement("external_pin");
			for (; ep; ep = ep->NextSiblingElement("external_pin"))
			{
				int pinId;
				PinConfig info;
				if (getExternalPinInfo(ep, pinId, info) == XML_SUCCESS)
				{
					mp_controller->initPin(info);
					m_dioMap[conId][pinId] = info;
				}
			}
		}
	}

    // Print the registers again after all the pins have been initialized.
    if (debug)
        mp_controller->printRegs();
    
    if (m_dioMap.size() <= 0)
    {
        mp_controller = nullptr;
        m_lastError = "XML Error: No valid connectors found";
        return false;
    }

    //Set the output mode of each dio if it's not already a valid mode.
    dioconfigmap_t::iterator it;
    for (it = m_dioMap.begin(); it != m_dioMap.end(); ++it)
    {
        pinconfigmap_t pinMap = it->second;
        //Not all units support programmable NPN/PNP modes so if these pins don't exist we don't really care.
        if (pinMap.find(ModeNpn) != pinMap.end() && pinMap.find(ModePnp) != pinMap.end())
        {
            PinConfig npn = pinMap[ModeNpn];
            PinConfig pnp = pinMap[ModePnp];

            try
            {
                //If these two pins are in the same state the dio will not operate. Let's fix that.
                if (mp_controller->getPinState(npn) == mp_controller->getPinState(pnp))
                {
                    mp_controller->setPinState(npn, true);
                    mp_controller->setPinState(pnp, false);
                }
            }
            catch (std::exception &ex) 
            {
                m_lastError = "DIO Controller Error: " + std::string(ex.what());
                return false;
            }
        }
    }

    return true;
}

diomap_t RsDioImpl::getPinList() const
{
    diomap_t dios;
    dioconfigmap_t::const_iterator dioIt;
    for (dioIt = m_dioMap.begin(); dioIt != m_dioMap.end(); ++dioIt)
    {
        pinmap_t pins;
        pinconfigmap_t::const_iterator pinIt;
        for (pinIt = dioIt->second.begin(); pinIt != dioIt->second.end(); ++pinIt)
        {
            pins[pinIt->first] = (PinInfo)pinIt->second;
        }

        dios[dioIt->first] = pins;
    }

    return dios;
}

int RsDioImpl::canSetOutputMode(int dio)
{
    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(ModeNpn) == pinMap.end() || pinMap.find(ModePnp) == pinMap.end())
    {
        return 0;
    }

    return 1;
}

int RsDioImpl::setOutputMode(int dio, OutputMode mode)
{
    if (mp_controller == nullptr)
    {
        m_lastError = "DIO Controller Error: Not initialized. Please run 'setXmlFile' first";
        return -1;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    if (mode == ModeError)
    {
        m_lastError = "Argument Error: Invalid mode 'ModeError'";
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(ModeNpn) == pinMap.end() || pinMap.find(ModePnp) == pinMap.end())
    {
        m_lastError = "Argument Error: Function not supported by dio " + std::to_string(dio);
        return -1;
    }

    try
    {
        mp_controller->setPinState(pinMap.at(ModeNpn), (mode == ModeNpn));
        mp_controller->setPinState(pinMap.at(ModePnp), (mode == ModePnp));
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

int RsDioImpl::digitalRead(int dio, int pin)
{
    if (mp_controller == nullptr)
    {
        m_lastError = "DIO Controller Error: Not initialized. Please run 'setXmlFile' first";
        return -1;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinConfig config = pinMap.at(pin);

    try 
    { 
        return mp_controller->getPinState(config);
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }
}

int RsDioImpl::digitalWrite(int dio, int pin, bool state)
{
    if (mp_controller == nullptr)
    {
        m_lastError = "DIO Controller Error: Not initialized. Please run 'setXmlFile' first";
        return -1;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinConfig config = pinMap.at(pin);
    if (!config.supportsOutput)
    {
        m_lastError = "Argument Error: Output mode not supported for pin " + std::to_string(pin);
        return -1;
    }

    try
    {
        mp_controller->setPinState(config, state);
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

std::string RsDioImpl::getLastError()
{
    std::string ret = m_lastError;
    m_lastError.clear();
    return ret;
}

RsDio *createRsDio()
{
    return new RsDioImpl;
}