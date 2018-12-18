#include "dio.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

#include <map>
#include <string>

#include <iostream>

static std::string s_lastError;
static AbstractDioController *sp_controller;

typedef std::map<int, PinInfo> pinmap_t;
static std::map<int, pinmap_t> s_dioMap;

bool initDio(const char* initFile)
{
    s_dioMap.clear();
    if (sp_controller) delete sp_controller;
    sp_controller = nullptr;
    rapidxml::xml_node<> *controllerNode = nullptr;
    rapidxml::xml_attribute<> *idAttr = nullptr;
    rapidxml::xml_node<> *connectorNode = nullptr;
    rapidxml::xml_node<> *pinNode = nullptr;
    rapidxml::xml_node<> *bitNode = nullptr;
    rapidxml::xml_node<> *gpioNode = nullptr;
    rapidxml::xml_node<> *invertNode = nullptr;
    rapidxml::xml_node<> *inputNode = nullptr;
    rapidxml::xml_node<> *outputNode = nullptr;
    
    try
    {
        rapidxml::xml_document<> doc;
        rapidxml::file<> file(initFile);
        doc.parse<0>(file.data());
        controllerNode = doc.first_node("dio_controller");
    }
    catch (std::exception &ex)
    {
        s_lastError = "XML Error: " + std::string(ex.what());
        return false;
    }

    if (!controllerNode)
    {
        s_lastError = "XML Error: No dio_controller found";
        return false;
    }

    idAttr = controllerNode->first_attribute("id");
    if (!idAttr)
    {
        s_lastError = "XML Error: No id attribute found for dio_controller";
        return false;
    }

    try
    {
        std::string s(idAttr->value());
        if (s == "ite8783")
            sp_controller = new Ite8783();
        else if (s == "ite8786")
            sp_controller = new Ite8786();
        else
        {
            s_lastError = "XML Error: Invalid id attribute found for dio_controller";
            return false;
        }
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return false;
    }
    
    connectorNode = controllerNode->first_node("connector");
    for (; connectorNode; connectorNode = connectorNode->next_sibling("connector"))
    {
        idAttr = connectorNode->first_attribute("id");
        if (idAttr)
        {
            int dioNumber = std::stoi(std::string(idAttr->value()));
            pinNode = connectorNode->first_node("internal_pin");
            for (; pinNode; pinNode = pinNode->next_sibling("internal_pin"))
            {
                idAttr = pinNode->first_attribute("id");
                bitNode = pinNode->first_node("bit_number");
                gpioNode = pinNode->first_node("gpio_number");
                if (idAttr && bitNode && gpioNode)
                {
                    int pinNumber = std::stoi(std::string(idAttr->value()));
                    uint8_t bitNumber = std::stoi(std::string(bitNode->value()));
                    uint8_t gpioNumber = std::stoi(std::string(gpioNode->value()));

                    PinInfo info(bitNumber, gpioNumber, false, false, true);
                    sp_controller->initPin(info);
                    s_dioMap[dioNumber][pinNumber] = info;
                }
            }

            pinNode = connectorNode->first_node("external_pin");
            for (; pinNode; pinNode = pinNode->next_sibling("external_pin"))
            {
                idAttr = pinNode->first_attribute("id");
                bitNode = pinNode->first_node("bit_number");
                gpioNode = pinNode->first_node("gpio_number");
                invertNode = pinNode->first_node("invert");
                inputNode = pinNode->first_node("input");
                outputNode = pinNode->first_node("output");
                if (idAttr && bitNode && gpioNode && invertNode && inputNode && outputNode)
                {
                    int pinNumber = std::stoi(std::string(idAttr->value()));
                    uint8_t bitNumber = std::stoi(std::string(bitNode->value()));
                    uint8_t gpioNumber = std::stoi(std::string(gpioNode->value()));
                    bool invert = (std::string(invertNode->value()) == "1");
                    bool input = (std::string(inputNode->value()) == "1");
                    bool output = (std::string(outputNode->value()) == "1");

                    PinInfo info(bitNumber, gpioNumber, invert, input, output);
                    sp_controller->initPin(info);
                    s_dioMap[dioNumber][pinNumber] = info;
                }
            }
        }
    }

    if (s_dioMap.size() <= 0)
    {
        sp_controller = nullptr;
        s_lastError = "XML Error: No connectors found";
        return false;
    }

    //Set the output mode of each dio if it's not already a valid mode.
    std::map<int, pinmap_t>::iterator it;
    for (it = s_dioMap.begin(); it != s_dioMap.end(); ++it)
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
                if (sp_controller->getPinState(npn) == sp_controller->getPinState(pnp))
                {
                    sp_controller->setPinState(npn, true);
                    sp_controller->setPinState(pnp, false);
                }
            }
            catch (std::exception &ex) 
            {
                s_lastError = "DIO Controller Error: " + std::string(ex.what());
                return false;
            }
        }
    }

    return true;
}

int digitalRead(int dio, int pin)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        s_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);

    try 
    { 
        return sp_controller->getPinState(info);
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }
}

int digitalWrite(int dio, int pin, bool state)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        s_lastError = "Argument Error: Invalid pin " + std::to_string(pin);
        return -1;
    }

    PinInfo info = pinMap.at(pin);
    if (!info.supportsOutput)
    {
        s_lastError = "Argument Error: Output mode not supported for pin " + std::to_string(pin);
        return -1;
    }

    try
    {
        sp_controller->setPinState(info, state);
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

int setOutputMode(int dio, OutputMode mode)
{
    if (sp_controller == nullptr)
        return -1;

    if (s_dioMap.find(dio) == s_dioMap.end())
    {
        s_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
        return -1;
    }

    pinmap_t pinMap = s_dioMap.at(dio);
    if (pinMap.find(ModeNpn) == pinMap.end() || pinMap.find(ModePnp) == pinMap.end())
    {
        s_lastError = "Argument Error: Function not supported by dio " + std::to_string(dio);
        return -1;
    }

    try
    {
        sp_controller->setPinState(pinMap.at(ModeNpn), (mode == ModeNpn));
        sp_controller->setPinState(pinMap.at(ModePnp), (mode == ModePnp));
    }
    catch (DioControllerError &ex)
    {
        s_lastError = "DIO Controller Error: " + std::string(ex.what());
        return -1;
    }

    return 0;
}

const char* getLastDioError()
{
    return s_lastError.c_str();
}