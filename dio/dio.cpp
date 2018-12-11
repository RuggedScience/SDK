#include "dio.h"
#include "controllers/ite8783.h"
#include "controllers/ite8786.h"
#include "../utils/rapidxml.hpp"
#include "../utils/rapidxml_utils.hpp"

Dio::Dio() :
    m_initFile(),
    m_lastErrorString(),
    mp_controller(nullptr)
{}

Dio::Dio(const char *initFile) :
    m_initFile(initFile),
    m_lastErrorString(),
    mp_controller(nullptr)
{}

Dio::Dio(std::string initFile) :
    m_initFile(initFile),
    m_lastErrorString(),
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

    rapidxml::xml_document<> doc;
    try
    {
        rapidxml::file<> file(m_initFile.c_str());
        doc.parse<0>(file.data());
    }
    catch (std::exception &ex)
    {
        setLastError("XML Error: " + std::string(ex.what()));
        return false;
    }

    rapidxml::xml_node<> *dioNode = doc.first_node("dio_controller");
    if (!dioNode)
    {
        setLastError("XML Error: No dio_controller found");
        return false;
    }

    rapidxml::xml_attribute<> *idAttr = dioNode->first_attribute("id");
    if (!idAttr)
    {
        setLastError("XML Error: No id attribute found for dio_controller");
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
            setLastError("XML Error: Invalid id attribute found for dio_controller");
            return false;
        }
    }
    catch (DioControllerError &ex)
    {
        setLastError("DIO Controller Error: " + std::string(ex.what()));
        return false;
    }
    
    rapidxml::xml_node<> *connectorNode = dioNode->first_node("connector");
    for (; connectorNode; connectorNode = connectorNode->next_sibling("connector"))
    {
        idAttr = connectorNode->first_attribute("id");
        if (idAttr)
        {
            int dioNumber = std::stoi(std::string(idAttr->value()));
            rapidxml::xml_node<> *pinNode = connectorNode->first_node("internal_pin");
            for (; pinNode; pinNode = pinNode->next_sibling("pin"))
            {
                idAttr = pinNode->first_attribute("id");
                rapidxml::xml_node<> *bitNode = pinNode->first_node("bit_number");
                rapidxml::xml_node<> *gpioNode = pinNode->first_node("gpio_number");
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
            for (; pinNode; pinNode = pinNode->next_sibling("pin"))
            {
                idAttr = pinNode->first_attribute("id");
                rapidxml::xml_node<> *bitNode = pinNode->first_node("bit_number");
                rapidxml::xml_node<> *gpioNode = pinNode->first_node("gpio_number");
                rapidxml::xml_node<> *invertNode = pinNode->first_node("invert");
                rapidxml::xml_node<> *inputNode = pinNode->first_node("input");
                rapidxml::xml_node<> *outputNode = pinNode->first_node("output");
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
        setLastError("XML Error: No connectors found");
        return false;
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
        setLastError("Argument Error: Invalid dio " + std::to_string(dio));
        return -1;
    }

    pinmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        setLastError("Argument Error: Invalid pin " + std::to_string(pin));
        return -1;
    }

    PinInfo info = pinMap.at(pin);

    try 
    { 
        return mp_controller->getPinState(info);
    }
    catch (DioControllerError &ex)
    {
        setLastError("DIO Controller Error: " + std::string(ex.what()));
        return -1;
    }
}

int Dio::digitalWrite(int dio, int pin, bool state)
{
    if (mp_controller == nullptr)
        return -1;

    if (m_dioMap.find(dio) == m_dioMap.end())
    {
        setLastError("Argument Error: Invalid dio " + std::to_string(dio));
        return -1;
    }

    pinmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(pin) == pinMap.end())
    {
        setLastError("Argument Error: Invalid pin " + std::to_string(pin));
        return -1;
    }

    PinInfo info = pinMap.at(pin);
    if (!info.supportsOutput)
    {
        setLastError("Argument Error: Output mode not supported for pin " + std::to_string(pin));
        return -1;
    }

    try
    {
        mp_controller->setPinState(info, state);
    }
    catch (DioControllerError &ex)
    {
        setLastError("DIO Controller Error: " + std::string(ex.what()));
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
        setLastError("Argument Error: Invalid dio " + std::to_string(dio));
        return -1;
    }

    pinmap_t pinMap = m_dioMap.at(dio);
    if (pinMap.find(ModeNpn) == pinMap.end() || pinMap.find(ModePnp) == pinMap.end())
    {
        setLastError("Argument Error: Function not supported by dio " + std::to_string(dio));
        return -1;
    }

    try
    {
        mp_controller->setPinState(pinMap.at(ModeNpn), (mode == ModeNpn));
        mp_controller->setPinState(pinMap.at(ModePnp), (mode == ModePnp));
    }
    catch (DioControllerError &ex)
    {
        setLastError("DIO Controller Error: " + std::string(ex.what()));
        return -1;
    }

    return 0;
}

std::string Dio::getLastErrorString()
{
    return m_lastErrorString;
}

void Dio::setLastError(const char *error)
{
    m_lastErrorString = std::string(error);
}

void Dio::setLastError(std::string error)
{
    m_lastErrorString = error;
}