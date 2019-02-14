#include "rsdio.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include "../utils/tinyxml2.h"

static tinyxml2::XMLError getInternalPinInfo(tinyxml2::XMLElement *pin, int& pinId, PinInfo& info)
{
	using namespace tinyxml2;

	int id, bit, gpio;
	XMLError e = pin->QueryAttribute("id", &id);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("bit", &bit);
	if (e != XML_SUCCESS) return e;
	e = pin->QueryAttribute("gpio", &gpio);
	if (e != XML_SUCCESS) return e;
	
	pinId = id;
	info = PinInfo(bit, gpio, false, false, true);
	return XML_SUCCESS;
}

static tinyxml2::XMLError getExternalPinInfo(tinyxml2::XMLElement *pin, int& pinId, PinInfo& info)
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
	info = PinInfo(bit, gpio, invert, input, output);
	return XML_SUCCESS;
}

RsDio::RsDio() :
    m_lastError(""),
    mp_controller(nullptr)
{}

RsDio::~RsDio()
{
    delete mp_controller;
}

void RsDio::destroy()
{
    delete this;
}

bool RsDio::setXmlFile(const char *fileName)
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
	    if (id == "ite8783")
    		mp_controller = new Ite8783();
    	else if (id == "ite8786")
		    mp_controller = new Ite8786();
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
				PinInfo info;
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
				PinInfo info;
				if (getExternalPinInfo(ep, pinId, info) == XML_SUCCESS)
				{
					mp_controller->initPin(info);
					m_dioMap[conId][pinId] = info;
				}
			}
		}
	}
    
    if (m_dioMap.size() <= 0)
    {
        mp_controller = nullptr;
        m_lastError = "XML Error: No valid connectors found";
        return false;
    }

    //Set the output mode of each dio if it's not already a valid mode.
    std::map<int, pinmap_t>::iterator it;
    for (it = m_dioMap.begin(); it != m_dioMap.end(); ++it)
    {
        pinmap_t pinMap = it->second;
        //Not all units support programmable NPN/PNP modes so if these pins don't exist we don't really care.
        if (pinMap.find(ModeNpn) != pinMap.end() && pinMap.find(ModePnp) != pinMap.end())
        {
            PinInfo npn = pinMap[ModeNpn];
            PinInfo pnp = pinMap[ModePnp];

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

int RsDio::digitalRead(int dio, int pin)
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

    pinmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);

    try 
    { 
        return mp_controller->getPinState(info);
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }
}

int RsDio::digitalWrite(int dio, int pin, bool state)
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

    pinmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);
    if (!info.supportsOutput)
    {
        m_lastError = "Argument Error: Output mode not supported for pin " + std::to_string(pin);
        return -1;
    }

    try
    {
        mp_controller->setPinState(info, state);
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

int RsDio::setOutputMode(int dio, OutputMode mode)
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

    pinmap_t pinMap = m_dioMap.at(dio);
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

const char *RsDio::getLastError()
{
    return m_lastError.c_str();
}

RsDioInterface *createRsDio()
{
    return new RsDio;
}