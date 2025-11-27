#include "rsparser.h"
#include "rserrors.h"


using namespace tinyxml2;

void RsXmlParser::parseImpl(XMLDocument &doc)
{
    const XMLElement* computerNode = doc.FirstChildElement("computer");
    if (!computerNode) 
        throw rs::RsException(rs::RsErrorCode::ConfigParseError, "Invalid config format");


    for (auto dioControllerNode : getChildNodes(computerNode, "dio_controller")) {
        DioControllerConfig controllerConfig;
        if (getDioControllerConfig(dioControllerNode, controllerConfig)) {
            addDioConfig(controllerConfig);
        }
    }
    
    for (auto poeControllerNode : getChildNodes(computerNode, "poe_controller")) {
        PoeControllerConfig controllerConfig;
        if (getPoeControllerConfig(poeControllerNode, controllerConfig)) {
            addPoeConfig(controllerConfig);
        }
    }
}

bool RsXmlParser::getRegisterConfig(const tinyxml2::XMLElement *registerNode, RegisterConfig &registerConfig) const
{
    const char *tmp = registerNode->Attribute("id");
    if (!tmp) return false;
    registerConfig.addr = std::stoi(std::string(tmp), nullptr, 0);

    tmp = registerNode->Attribute("ldn");
    registerConfig.ldn = tmp ? std::stoi(std::string(tmp), nullptr, 0) : 0;
    
    tmp = registerNode->Attribute("onBits");
    registerConfig.onBits = tmp ? std::stoi(std::string(tmp), nullptr, 0) : 0;

    tmp = registerNode->Attribute("offBits");
    registerConfig.offBits = tmp ? std::stoi(std::string(tmp), nullptr, 0) : 0;
    
    return true;
}

bool RsXmlParser::getDioControllerConfig(const XMLElement *controllerNode, DioControllerConfig &controllerConfig) const
{
    const char *id = controllerNode->Attribute("id");
    if (!id) return false;

    controllerConfig.controllerId = std::string(id);

    for (auto registerNode : getChildNodes(controllerNode, "register")) {
        RegisterConfig registerConfig;
        if (getRegisterConfig(registerNode, registerConfig)) {
            controllerConfig.registers.push_back(registerConfig);
        }
    }

    for (auto connectorNode : getChildNodes(controllerNode, "connector")) {
        DioConnectorConfig connectorConfig;
        if (getDioConnectorInfo(connectorNode, connectorConfig)) {
            controllerConfig.connectors.push_back(connectorConfig);
        }

    }

    return true;
}

bool RsXmlParser::getDioConnectorInfo(const XMLElement *connectorNode, DioConnectorConfig &connectorConfig) const
{
    int connectorId = 0;
    if (connectorNode->QueryAttribute("id", &connectorId) != XML_SUCCESS) return false;

    connectorConfig.connectorId = connectorId;

    for (auto internalPinNode : getChildNodes(connectorNode, "internal_pin")) {
        DioPinConfig info;
        if (getInternalPinInfo(internalPinNode, info)) {
            if (info.id == -1) {
                connectorConfig.sourcePin = info;
            } else if (info.id == -2) {
                connectorConfig.sinkPin = info;
            }
        }
    }

    for (auto externalPinNode : getChildNodes(connectorNode, "external_pin")) {
        DioPinConfig info;
        if (getExternalPinInfo(externalPinNode, info)) {
            connectorConfig.pins.push_back(info);
        }
    }

    return true;
}

bool RsXmlParser::getInternalPinInfo(const XMLElement* pinNode, DioPinConfig& pinConfig) const
{
    int id, bit, gpio;
    bool pullup = false;
    XMLError e = pinNode->QueryAttribute("id", &id);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("bit", &bit);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("gpio", &gpio);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("pullup", &pullup);
    if (e != XML_SUCCESS && e != XML_NO_ATTRIBUTE) return false;

    pinConfig.id = id;
    pinConfig.bitmask = 1 << bit;
    pinConfig.offset = gpio - 1;
    pinConfig.invert = false;
    pinConfig.enablePullup = pullup;
    pinConfig.supportsInput = false;
    pinConfig.supportsOutput = true;
    return true;
}

bool RsXmlParser::getExternalPinInfo(const XMLElement* pinNode, DioPinConfig& pinConfig) const
{
    int id, bit, gpio;
    bool invert, input, output, pullup = false;
    XMLError e = pinNode->QueryAttribute("id", &id);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("bit", &bit);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("gpio", &gpio);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("invert", &invert);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("input", &input);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("output", &output);
    if (e != XML_SUCCESS) return false;
    e = pinNode->QueryAttribute("pullup", &pullup);
    if (e != XML_SUCCESS && e != XML_NO_ATTRIBUTE) return false;

    pinConfig.id = id;
    pinConfig.bitmask = 1 << bit;
    pinConfig.offset = gpio - 1;
    pinConfig.invert = invert;
    pinConfig.enablePullup = pullup;
    pinConfig.supportsInput = input;
    pinConfig.supportsOutput = output;
    return true;
}

bool RsXmlParser::getPoeControllerConfig(const XMLElement *controllerNode, PoeControllerConfig &controllerConfig) const
{
    const char *attr = controllerNode->Attribute("id");
    if (!attr) return false;

    controllerConfig.controllerId = std::string(attr);

    // Try to load the new XML version with the chip_address attribute
    attr = controllerNode->Attribute("chip_address");
    if (!attr) {
        // If it doesn't work try the old XML version.
        attr = controllerNode->Attribute("address");
        if (!attr) {
            return false;
        }
    }

    controllerConfig.chipAddr = std::stoi(std::string(attr), nullptr, 0);
    
    attr = controllerNode->Attribute("bus_address");
    // Default to the old bus address
    // We don't throw an error when the bus address is missing.
    // This is to keep old XML files working.
    controllerConfig.busAddr = attr ? std::stoi(std::string(attr), nullptr, 0) : 0xF040;

    for (auto portNode : getChildNodes(controllerNode, "port")) {
        PoePortConfig portConfig;
        if (getPoePortConfig(portNode, portConfig)) {
            controllerConfig.ports.push_back(portConfig);
        }
    }

    return true;
}

bool RsXmlParser::getPoePortConfig(const XMLElement *portNode, PoePortConfig &portConfig) const
{
    int id, bit;
    if (portNode->QueryAttribute("id", &id) != XML_SUCCESS) return false;
    if (portNode->QueryAttribute("bit", &bit) != XML_SUCCESS) return false;
    
    portConfig.id = id;
    portConfig.offset = bit;
    return true;
}
