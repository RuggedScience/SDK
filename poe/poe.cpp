#include "poe.h"
#include "controllers/pd69104.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

#include <string>

Poe::Poe() :
    m_initFile(),
    m_lastError(NoError),
    m_lastErrorString(),
    mp_controller(nullptr)
{}

Poe::Poe(const char *initFile) :
    m_initFile(initFile),
    m_lastError(NoError),
    m_lastErrorString(),
    mp_controller(nullptr)
{}

Poe::Poe(std::string initFile) :
    m_initFile(initFile),
    m_lastError(NoError),
    m_lastErrorString(),
    mp_controller(nullptr)
{}

Poe::~Poe()
{
    if (mp_controller)
        delete mp_controller;
}

bool Poe::open()
{
    m_portMap.clear();
    mp_controller = nullptr;
    rapidxml::xml_document<> doc;

    try
    {
        rapidxml::file<> file(m_initFile.c_str());
        doc.parse<0>(file.data());
    }
    catch (std::exception &ex)
    {
        setLastError(XmlError, ex.what());
        return false;
    }

    rapidxml::xml_node<> *node = doc.first_node("poe_controller");
    if (!node)
    {
        setLastError(XmlError, "No poe_controller found in XML file");
        return false;
    }
    
    rapidxml::xml_attribute<> *idAttr = node->first_attribute("id");
    if (!idAttr)
    {
        setLastError(XmlError, "No id attribute found for poe_controller");
        return false;
    }

    try
    {
        std::string s(idAttr->value());
        if (s == "pd69104")
            mp_controller = new Pd69104();
        else
        {
            setLastError(XmlError, "Invalid poe_controller found in XML file");
            return false;
        }
    }
    catch (PoeControllerError &ex)
    {
        setLastError(ControllerError, ex.what());
        return false;
    }

    rapidxml::xml_node<> *port = node->first_node("port");
    for (; port; port = port->next_sibling("port"))
    {
        idAttr = port->first_attribute("id");
        rapidxml::xml_node<> *bitNode = port->first_node("bit_number");
        if (idAttr && bitNode)
        {
            int portNumber = std::stoi(std::string(idAttr->value()));
            uint8_t bitNumber = std::stoi(std::string(bitNode->value()));
            m_portMap[portNumber] = bitNumber;
        }
    }

    if (m_portMap.size() <= 0)
    {
        mp_controller = nullptr;
        setLastError(XmlError, "No PoE ports found in XML file");
        return false;
    }

    return true;
}

bool Poe::open(const char *initFile)
{
    m_initFile = std::string(initFile);
    return open();
}

bool Poe::open(std::string initFile)
{
    m_initFile = initFile;
    return open();
}

Poe::PoeState Poe::getPortState(int port)
{
    if (mp_controller == nullptr)
        return StateError;

    if (m_portMap.find(port) == m_portMap.end())
    {
        setLastError(PortError, "Invalid Port");
        return StateError;
    }

    try 
    { 
        return mp_controller->getPortState(m_portMap[port]); 
    }
    catch (PoeControllerError &ex) 
    {
        setLastError(ControllerError, ex.what());
    }

    return StateError;
}

bool Poe::setPortState(int port, PoeState state)
{
    if (mp_controller == nullptr)
        return false;

    if (m_portMap.find(port) == m_portMap.end())
    {
        setLastError(PortError, "Invalid Port");
        return false;
    }

    try
    {
        mp_controller->setPortState(m_portMap[port], state);
    }
    catch (PoeControllerError &ex)
    {
        setLastError(ControllerError, ex.what());
        return false;
    }

    return true;
}

Poe::PoeError Poe::getLastError()
{
    return m_lastError;
}

std::string Poe::getLastErrorString()
{
    return m_lastErrorString;
}

void Poe::setLastError(PoeError error, const char *string)
{
    m_lastError = error;
    m_lastErrorString = std::string(string);
}

void Poe::setLastError(PoeError error, std::string string)
{
    m_lastError = error;
    m_lastErrorString = string;
}