#include "rspoeimpl.h"
#include "controllers/pd69104.h"
#include "controllers/pd69200.h"
#include "controllers/ltc4266.h"

#include <tinyxml2.h>
#include <rserrors.h>

#include <iostream>

RsPoeImpl::RsPoeImpl() 
    : m_lastError()
    , m_lastErrorString()
    , mp_controller(nullptr)
{}

RsPoeImpl::RsPoeImpl(AbstractPoeController *controller, portmap_t portMap)
    : m_lastError()
    , m_lastErrorString()
    , m_portMap(portMap)
    , mp_controller(controller)
{}

RsPoeImpl::~RsPoeImpl()
{
    delete mp_controller;
}

void RsPoeImpl::destroy()
{
    delete this;
}

bool RsPoeImpl::setXmlFile(const char *fileName)
{
    using namespace tinyxml2;
    m_portMap.clear();
    delete mp_controller;
    mp_controller = nullptr;

    XMLDocument doc;
    if (doc.LoadFile(fileName) != XML_SUCCESS)
    {
        if (doc.ErrorID() == XML_ERROR_FILE_NOT_FOUND)
        {
            m_lastError = std::make_error_code(std::errc::no_such_file_or_directory);
            m_lastErrorString = std::string(fileName) + " not found";
        }
        else
        {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = doc.ErrorStr();
        }
        
        return false;
    }

    XMLElement *comp = doc.FirstChildElement("computer");
    if (!comp)
    {
        m_lastError = RsErrorCode::XmlParseError;
        m_lastErrorString = "Missing computer node";
        return false;
    }

    XMLElement *poe = comp->FirstChildElement("poe_controller");
    if (!poe)
    {
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "PoE functionality not supported";
        return false;
    }

    std::string id(poe->Attribute("id"));

    // Try to load the new XML version with the chip_address attribute
    const char * chipAddressStr = poe->Attribute("chip_address");
    if (!chipAddressStr)
    {
        // If it doesn't work try the old XML version.
        chipAddressStr = poe->Attribute("address");
        if (!chipAddressStr)
        {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = "Missing address attribute for poe_controller";
            return false;
        }
    }
    int chipAddress = std::stoi(std::string(chipAddressStr), nullptr, 0);
    
    // Default to the old bus address
    // We don't throw an error when the bus address is missing.
    // This is to keep old XML files working.
    int busAddress = 0xF040;
    if (busAddress)
    {
        busAddress = std::stoi(std::string(poe->Attribute("bus_address")), nullptr, 0);
    }

    try
    {
        if (id == "pd69104")
            mp_controller = new Pd69104(busAddress, chipAddress);
        else if (id == "pd69200")
            mp_controller = new Pd69200(busAddress, chipAddress);
        else if (id == "ltc4266")
            mp_controller = new Ltc4266(busAddress, chipAddress);
        else
        {
            m_lastError = RsErrorCode::XmlParseError;
            m_lastErrorString = "Invalid PoE controller ID";
            return false;
        }
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
        return false;
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
        return false;
    }

    XMLElement *port = poe->FirstChildElement("port");
    for (; port; port = port->NextSiblingElement("port"))
    {
        int id, bit;
        if (port->QueryAttribute("id", &id) != XML_SUCCESS)
            continue;

        if (port->QueryAttribute("bit", &bit) != XML_SUCCESS)
            continue;

        m_portMap[id] = bit;
    }

    if (m_portMap.size() <= 0)
    {
        m_portMap.clear();
        delete mp_controller;
        mp_controller = nullptr;
        
        m_lastError = std::make_error_code(std::errc::function_not_supported);
        m_lastErrorString = "PoE function not supported";
        return false;
    }

    return true;
}

rs::PoeState RsPoeImpl::getPortState(int port)
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return rs::PoeState::Error;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return rs::PoeState::Error;
    }

    try 
    { 
        return mp_controller->getPortState(m_portMap[port]); 
    }
    catch (const std::system_error &ex) 
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return rs::PoeState::Error;
}

int RsPoeImpl::setPortState(int port, rs::PoeState state)
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return -1;
    }

    try
    {
        mp_controller->setPortState(m_portMap[port], state);
        return 0;
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1;
}

float RsPoeImpl::getPortVoltage(int port)
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1.0f;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return -1.0f;
    }
    
    try
    {
        return mp_controller->getPortVoltage(m_portMap[port]);
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1.0f;
}

float RsPoeImpl::getPortCurrent(int port)
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1.0f;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return -1.0f;
    }

    try
    {
        return mp_controller->getPortCurrent(m_portMap[port]);
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1.0f;
}

float RsPoeImpl::getPortPower(int port)
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1.0f;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = std::make_error_code(std::errc::invalid_argument);
        m_lastErrorString = "Invalid port";
        return -1.0f;
    }

    try
    {
        return mp_controller->getPortPower(m_portMap[port]);
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1.0f;
}

int RsPoeImpl::getBudgetConsumed()
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1;
    }

    try
    {
        return mp_controller->getBudgetConsumed();
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1;
}

int RsPoeImpl::getBudgetAvailable()
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1;
    }

    try
    {
        return mp_controller->getBudgetAvailable();
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1;
}

int RsPoeImpl::getBudgetTotal()
{
    if (mp_controller == nullptr)
    {
        m_lastError = RsErrorCode::NotInitialized;
        m_lastErrorString = "XML file never set";
        return -1;
    }

    try
    {
        return mp_controller->getBudgetTotal();
    }
    catch (const std::system_error &ex)
    {
        m_lastError = ex.code();
        m_lastErrorString = ex.what();
    }
    catch (...)
    {
        m_lastError = RsErrorCode::UnknownError;
        m_lastErrorString = "Unknown exception occurred";
    }

    return -1;
}

std::error_code RsPoeImpl::getLastError() const
{
    return m_lastError;
}

std::string RsPoeImpl::getLastErrorString() const
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

rs::RsPoe *rs::createRsPoe()
{
    return new RsPoeImpl;
}

const char *rs::rsPoeVersion()
{ 
    return RSPOE_VERSION_STRING;
}
