#include "poe.h"
#include "controllers/pd69104.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

#include <map>
#include <string>
#include <stdint.h>

std::string s_lastError;
AbstractPoeController *sp_controller = nullptr;
std::map<int, uint8_t> s_portMap;

bool init(const char* initFile)
{
    s_portMap.clear();
    if (sp_controller) delete sp_controller;
    sp_controller = nullptr;
    rapidxml::xml_node<> *controllerNode = nullptr;
    rapidxml::xml_attribute<> *idAttr = nullptr;
    rapidxml::xml_node<> *portNode = nullptr;
    rapidxml::xml_node<> *bitNode = nullptr;

    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> file(initFile);
        doc.parse<0>(file.data());
        controllerNode = doc.first_node("poe_controller");

    }
    catch (rapidxml::parse_error &ex)
    {
        s_lastError = "XML Error: " + std::string(ex.what()) + "\n" + std::string(ex.where<char>());
        return false;
    }
    catch (std::exception &ex)
    {
        s_lastError = "XML Error: " + std::string(ex.what());
        return false;
    }

    if (!controllerNode)
    {
        s_lastError = "XML Error: No poe_controller found";
        return false;
    }
    
    idAttr = controllerNode->first_attribute("id");
    if (!idAttr)
    {
        s_lastError = "XML Error: No id attribute found for poe_controller";
        return false;
    }

    try
    {
        std::string s(idAttr->value());
        if (s == "pd69104")
            sp_controller = new Pd69104(0xF040, 0x40);
        else
        {
            s_lastError = "XML Error: Invalid poe_controller found";
            return false;
        }
    }
    catch (PoeControllerError &ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
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
            s_portMap[portNumber] = bitNumber;
        }
    }

    if (s_portMap.size() <= 0)
    {
        sp_controller = nullptr;
        s_lastError = "XML Error: No ports found";
        return false;
    }

    return true;
}

PoeState getPortState(int port)
{
    if (sp_controller == nullptr)
        return StateError;

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return StateError;
    }

    try 
    { 
        return sp_controller->getPortState(s_portMap[port]); 
    }
    catch (PoeControllerError &ex) 
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
    }

    return StateError;
}

int getPortState(int port, PoeState state)
{
    if (sp_controller == nullptr)
        return false;

    if (s_portMap.find(port) == s_portMap.end())
    {
        s_lastError = "Argument Error: Invalid port " + std::to_string(port);
        return false;
    }

    try
    {
        sp_controller->setPortState(s_portMap[port], state);
    }
    catch (PoeControllerError &ex)
    {
        s_lastError = "PoE Controller Error: " + std::string(ex.what());
        return false;
    }

    return true;
}

const char* getLastError()
{
    return s_lastError.c_str();
}