#ifndef RSPARSER_H
#define RSPARSER_H

#include "xmlparser.h"
#include "tinyxml2.h"

class RsXmlParser : public XmlParser {
   protected:
    void parseImpl(tinyxml2::XMLDocument &doc) override;

   private:
    bool getRegisterConfig(
        const tinyxml2::XMLElement* registerNode,
        RegisterConfig& registerConfig
    ) const;

    bool getDioControllerConfig(
        const tinyxml2::XMLElement* controllerNode,
        DioControllerConfig& controllerConfig
    ) const;
    bool getDioConnectorInfo(
        const tinyxml2::XMLElement* connectorNode,
        DioConnectorConfig& connectorConfig
    ) const;
    bool getInternalPinInfo(
        const tinyxml2::XMLElement* pinNode,
        DioPinConfig& pinConfig
    ) const;
    bool getExternalPinInfo(
        const tinyxml2::XMLElement* pinNode,
        DioPinConfig& pinConfig
    ) const;

    bool getPoeControllerConfig(
        const tinyxml2::XMLElement* controllerNode,
        PoeControllerConfig& controllerConfig
    ) const;
    bool getPoePortConfig(
        const tinyxml2::XMLElement* portElement,
        PoePortConfig& portConfig
    ) const;
};

#endif  // RSPARSER_H