#include "rspoeimpl.h"
#include "controllers/pd69104.h"
#include "controllers/pd69200.h"
#include "controllers/ltc4266.h"
#include "../utils/tinyxml2.h"

RsPoeImpl::RsPoeImpl() :
    m_lastError(""),
    mp_controller(nullptr)
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
        m_lastError = "XML Error: Unable to load file";
        return false;
    }

    XMLElement *comp = doc.FirstChildElement("computer");
    if (!comp)
    {
        m_lastError = "XML Error: Unable to find computer node";
        return false;
    }

    XMLElement *poe = comp->FirstChildElement("poe_controller");
    if (!poe)
    {
        m_lastError = "XML Error: Unable to find poe_controller node";
        return false;
    }

    std::string id(poe->Attribute("id"));
    int address = std::stoi(std::string(poe->Attribute("address")), nullptr, 0);
    try
    {
        if (id == "pd69104")
            mp_controller = new Pd69104(0xF040, address);
        else if (id == "pd69200")
            mp_controller = new Pd69200(0xF040, address);
        else if (id == "ltc4266")
            mp_controller = new Ltc4266(0xF040, address);
        else
        {
            m_lastError = "XML Error: Invalid id found for poe_controller";
            return false;
        }
    }
    catch (std::exception &ex)
    {
        m_lastError = "POE Controller Error: " + std::string(ex.what());
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
        mp_controller = nullptr;
        m_lastError = "XML Error: No ports found";
        return false;
    }

    return true;
}

PoeState RsPoeImpl::getPortState(int port)
{
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return StateError;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return StateError;
    }

    try 
    { 
        return mp_controller->getPortState(m_portMap[port]); 
    }
    catch (PoeControllerError &ex) 
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
    }

    return StateError;
}

int RsPoeImpl::setPortState(int port, PoeState state)
{
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return -1;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return -1;
    }

    if (state == StateError)
    {
        m_lastError = "Argument Error: Invalid state 'StateError'";
        return -1;
    }

    try
    {
        mp_controller->setPortState(m_portMap[port], state);
    }
    catch (PoeControllerError &ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

float RsPoeImpl::getPortVoltage(int port)
{
    float volts = -1.0f;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return volts;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return volts;
    }
    
    try
    {
        volts = mp_controller->getPortVoltage(m_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return volts;
    }

    return volts;
}

float RsPoeImpl::getPortCurrent(int port)
{
    float cur = -1.0f;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return cur;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return cur;
    }

    try
    {
        cur = mp_controller->getPortCurrent(m_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return cur;
    }

    return cur;
}

float RsPoeImpl::getPortPower(int port)
{
    float power = -1.0f;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return power;
    }

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return power;
    }

    try
    {
        power = mp_controller->getPortPower(m_portMap[port]);
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return power;
    }

    return power;
}

int RsPoeImpl::getBudgetConsumed()
{
    int consumed = -1;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return consumed;
    }

    try
    {
        consumed = mp_controller->getBudgetConsumed();
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return consumed;
    }

    return consumed;
}

int RsPoeImpl::getBudgetAvailable()
{
    int available = -1;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return available;
    }

    try
    {
        available = mp_controller->getBudgetAvailable();
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return available;
    }

    return available;
}

int RsPoeImpl::getBudgetTotal()
{
    int total = -1;
    if (mp_controller == nullptr)
    {
        m_lastError = "POE Controller Error: Not initialized. Please run 'initPoe' first";
        return total;
    }

    try
    {
        total = mp_controller->getBudgetTotal();
    }
    catch (PoeControllerError& ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return total;
    }

    return total;
}

std::string RsPoeImpl::getLastError()
{
    std::string ret = m_lastError;
    m_lastError.clear();
    return ret;
}

RsPoe *createRsPoe()
{
    return new RsPoeImpl;
}