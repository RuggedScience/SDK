#include "poe.h"
#include "controllers/pd69104.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

#include <string>

Poe::Poe() :
    m_initFile(),
    m_lastError(),
    mp_controller(nullptr)
{}

Poe::Poe(const char *initFile) :
    m_initFile(initFile),
    m_lastError(),
    mp_controller(nullptr)
{}

Poe::Poe(std::string initFile) :
    m_initFile(initFile),
    m_lastError(),
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
    rapidxml::xml_node<> *controllerNode = nullptr;
    rapidxml::xml_attribute<> *idAttr = nullptr;
    rapidxml::xml_node<> *portNode = nullptr;
    rapidxml::xml_node<> *bitNode = nullptr;

    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> file(m_initFile.c_str());
        doc.parse<0>(file.data());
        controllerNode = doc.first_node("poe_controller");

    }
    catch (rapidxml::parse_error &ex)
    {
        
        m_lastError = "XML Error: " + std::string(ex.what()) + "\n" + std::string(ex.where<char>());
        return false;
    }
    catch (std::exception &ex)
    {
        m_lastError = "XML Error: " + std::string(ex.what());
        return false;
    }

    if (!controllerNode)
    {
        m_lastError = "XML Error: No poe_controller found";
        return false;
    }
    
    idAttr = controllerNode->first_attribute("id");
    if (!idAttr)
    {
        m_lastError = "XML Error: No id attribute found for poe_controller";
        return false;
    }

    try
    {
        std::string s(idAttr->value());
        if (s == "pd69104")
            mp_controller = new Pd69104();
        else
        {
            m_lastError = "XML Error: Invalid poe_controller found";
            return false;
        }
    }
    catch (PoeControllerError &ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return false;
    }

    portNode = controllerNode->first_node("port");
    for (; portNode; portNode = portNode->next_sibling("port"))
    {
        idAttr = portNode->first_attribute("id");
        bitNode = portNode->first_node("bit_number");
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
        m_lastError = "XML Error: No ports found";
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

int Poe::setPortState(int port, PoeState state)
{
    if (mp_controller == nullptr)
        return false;

    if (m_portMap.find(port) == m_portMap.end())
    {
        m_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return false;
    }

    try
    {
        mp_controller->setPortState(m_portMap[port], state);
    }
    catch (PoeControllerError &ex)
    {
        m_lastError = "PoE Controller Error: " + std::string(ex.what());
        return false;
    }

    return true;
}

std::string Poe::getLastError()
{
    return m_lastError;
}