#include "dio.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

Dio::Dio() :
    m_initFile(),
    m_lastError(),
    mp_controller(nullptr)
{}

Dio::Dio(const char *initFile) :
    m_initFile(initFile),
    m_lastError(),
    mp_controller(nullptr)
{}

Dio::Dio(std::string initFile) :
    m_initFile(initFile),
    m_lastError(),
    mp_controller(nullptr)
{}

Dio::~Dio()
{
    if (mp_controller)
        delete mp_controller;
}

bool Dio::open()
{
    m_dioMap.clear();
    mp_controller = nullptr;
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
        rapidxml::file<> file(m_initFile.c_str());
        doc.parse<0>(file.data());
        controllerNode = doc.first_node("dio_controller");
    }
    catch (std::exception &ex)
    {
        m_lastError = "XML Error: " + std::string(ex.what());
        return false;
    }

    if (!controllerNode)
    {
        m_lastError = "XML Error: No dio_controller found";
        return false;
    }

    idAttr = controllerNode->first_attribute("id");
    if (!idAttr)
    {
        m_lastError = "XML Error: No id attribute found for dio_controller";
        return false;
    }

    try
    {
        std::string s(idAttr->value());
        if (s == "ite8783")
            mp_controller = new Ite8783();
        else if (s == "ite8786")
            mp_controller = new Ite8786();
        else
        {
            m_lastError = "XML Error: Invalid id attribute found for dio_controller";
            return false;
        }
    }
    catch (DioControllerError &ex)
    {
        m_lastError = "DIO Controller Error: " + std::string(ex.what());
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
                    mp_controller->initPin(info);
                    m_dioMap[dioNumber][pinNumber] = info;
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
                    mp_controller->initPin(info);
                    m_dioMap[dioNumber][pinNumber] = info;
                }
            }
        }
    }

    if (m_dioMap.size() <= 0)
    {
        mp_controller = nullptr;
        m_lastError = "XML Error: No connectors found";
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

bool Dio::open(const char *initFile)
{
    m_initFile = std::string(initFile);
    return open();
}

bool Dio::open(std::string initFile)
{
    m_initFile = initFile;
    return open();
}

int Dio::digitalRead(int dio, int pin)
{
    if (mp_controller == nullptr)
        return -1;

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

int Dio::digitalWrite(int dio, int pin, bool state)
{
    if (mp_controller == nullptr)
        return -1;

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

int Dio::setOutputMode(int dio, OutputMode mode)
{
    if (mp_controller == nullptr)
        return -1;

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        m_lastError = "Argument Error: Invalid dio " + std::to_string(dio);
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

std::string Dio::getLastError()
{
    return m_lastError;
}