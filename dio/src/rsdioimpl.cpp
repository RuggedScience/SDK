#include <iostream>
#include <assert.h>

#include "rssdk_errors.hpp"
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

RsDioImpl::RsDioImpl() 
    : m_lastError()
    , m_lastErrorString()
    , mp_controller(nullptr)
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
        if (doc.ErrorID() == XML_ERROR_FILE_NOT_FOUND)
        {
            m_lastError = std::make_error_code(std::errc::no_such_file_or_directory);
            m_lastErrorString = fileName;
        }
        else
        {
            m_lastError = rs::RsSdkError::XmlParseError;
            m_lastErrorString = doc.ErrorStr();
        }
        
        return false;
    }

	XMLElement *comp = doc.FirstChildElement("computer");
	if (!comp)
	{
        m_lastError = rs::RsSdkError::XmlParseError;
        m_lastErrorString = "Missing computer node";
        return false;
    }

	XMLElement *dio = comp->FirstChildElement("dio_controller");
	if (!dio)
	{
        m_lastError = rs::RsSdkError::XmlParseError;
        m_lastErrorString = "Missing dio_controller node";
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
    		m_lastError = rs::RsSdkError::XmlParseError;
            m_lastErrorString = "Invalid DIO controller ID";
		    return false;
	    }
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        if (ex.code() == rs::RsSdkError::PermissionDenied)
            m_lastErrorString = "Must be run as root";
        else
            m_lastErrorString = ex.what();

        return false;
    }
    catch (...)
    {
        m_lastError = rs::RsSdkError::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
        return false;
    }

    // Print the registers before we initialize all the pins.
    if (debug)
    {
        mp_controller->printRegs();
    }

	XMLElement *con = dio->FirstChildElement("connector");
    // No connectors means this unit doesn't support DIO.
    // Assuming it's a legit XML file... 
	if (!con)
	{
        m_dioMap.clear();
        delete mp_controller;
        mp_controller = nullptr;

        m_lastError = rs::RsSdkError::FunctionNotSupported;
        m_lastErrorString = "DIO function not supported";
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
        m_dioMap.clear();
        delete mp_controller;
        mp_controller = nullptr;

        m_lastError = rs::RsSdkError::XmlParseError;
        m_lastErrorString = "Found DIO connector node but no pins";
        return false;
    }

    //Set the output mode of each dio if it's not already a valid mode.
    dioconfigmap_t::iterator it;
    for (it = m_dioMap.begin(); it != m_dioMap.end(); ++it)
    {
        const int modeSink = static_cast<int>(rs::OutputMode::Sink);
        const int modeSource = static_cast<int>(rs::OutputMode::Source);

        pinconfigmap_t pinMap = it->second;
        //Not all units support programmable Source/Sink modes so if these pins don't exist we don't really care.
        if (pinMap.find(modeSink) != pinMap.end() && pinMap.find(modeSource) != pinMap.end())
        {
            PinConfig sink = pinMap[modeSink];
            PinConfig source = pinMap[modeSource];

            try
            {
                //If these two pins are in the same state the dio will not operate. Let's fix that.
                if (mp_controller->getPinState(sink) == mp_controller->getPinState(source))
                {
                    mp_controller->setPinState(sink, true);
                    mp_controller->setPinState(source, false);
                }
            }
            catch (const std::system_error &ex) 
            {
                m_dioMap.clear();
                delete mp_controller;
                mp_controller = nullptr;

                m_lastError = ex.code();
                m_lastErrorString = ex.what();
                return false;
            }
        }
    }

    return true;
}

rs::diomap_t RsDioImpl::getPinList() const
{
    rs::diomap_t dios;
    dioconfigmap_t::const_iterator dioIt;
    for (dioIt = m_dioMap.begin(); dioIt != m_dioMap.end(); ++dioIt)
    {
        rs::pinmap_t pins;
        pinconfigmap_t::const_iterator pinIt;
        for (pinIt = dioIt->second.begin(); pinIt != dioIt->second.end(); ++pinIt)
        {
            pins[pinIt->first] = (rs::PinInfo)pinIt->second;
        }

        dios[dioIt->first] = pins;
    }

    return dios;
}

int RsDioImpl::canSetOutputMode(int dio)
{
    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid DIO";
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    if (pinMap.find(modeSink) == pinMap.end() || pinMap.find(modeSource) == pinMap.end())
    {
        return 0;
    }

    return 1;
}

bool RsDioImpl::setOutputMode(int dio, rs::OutputMode mode)
{
    if (mp_controller == nullptr)
    {
        m_lastError = rs::RsSdkError::NotInitialized;
        m_lastErrorString = "XML file never set";
        return false;
    }

    if (mode == rs::OutputMode::Error)
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid output mode";
        return false;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid DIO";
        return false;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    const int modeSink = static_cast<int>(rs::OutputMode::Sink);
    const int modeSource = static_cast<int>(rs::OutputMode::Source);
    if (pinMap.find(modeSink) == pinMap.end() || pinMap.find(modeSource) == pinMap.end())
    {
        m_lastError = rs::RsSdkError::FunctionNotSupported;
        m_lastErrorString = "Setting output mode not supported";
        return false;
    }

    try
    {
        mp_controller->setPinState(pinMap.at(modeSink), (mode == rs::OutputMode::Sink));
        mp_controller->setPinState(pinMap.at(modeSource), (mode == rs::OutputMode::Source));
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
        return false;
    } 
    catch (...)
    {
        m_lastError = rs::RsSdkError::UnknownError;
        m_lastErrorString = "Unknown exception occured";
    }

    return true;
}

int RsDioImpl::digitalRead(int dio, int pin)
{
    if (mp_controller == nullptr)
    {
        m_lastError = rs::RsSdkError::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid DIO";
        return -1;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid pin";
        return -1;
    }

    PinConfig config = pinMap.at(pin);

    try 
    { 
        return mp_controller->getPinState(config);
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = rs::RsSdkError::UnknownError;
        m_lastErrorString = "unknown exception occured";
    }

    return -1;
}

bool RsDioImpl::digitalWrite(int dio, int pin, bool state)
{
    if (mp_controller == nullptr)
    {
        m_lastError = rs::RsSdkError::NotInitialized;
        m_lastErrorString = "XML file never set";
        return false;
    }

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid DIO";
        return false;
    }

    pinconfigmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        m_lastError = rs::RsSdkError::InvalidArgument;
        m_lastErrorString = "Invalid pin";
        return false;
    }

    PinConfig config = pinMap.at(pin);

    try
    {
        mp_controller->setPinState(config, state);
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
        return false;
    }

    return true;
}

std::error_code RsDioImpl::getLastError() const
{
    return m_lastError;
}

std::string RsDioImpl::getLastErrorString() const
{
    std::string lastError;

    if (m_lastError)
    {
        lastError += m_lastError.message();
        if (!m_lastErrorString.empty())
        {
            lastError += ": " + m_lastErrorString;
        }
    }
    return lastError;
}

rs::RsDio *rs::createRsDio()
{
    return new RsDioImpl;
}

const char *rs::rsDioVersion()
{ 
    return RSDIO_VERSION_STRING;
}
