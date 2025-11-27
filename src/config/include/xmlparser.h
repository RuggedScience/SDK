#ifndef XMLPARSER_H
#define XMLPARSER_H

#include "abstractconfigparser.h"
#include "tinyxml2.h"

class XmlParser : public AbstractConfigParser {
   protected:
    void parseImpl(const char* fileName) override;
    virtual void parseImpl(tinyxml2::XMLDocument& document) = 0;
    std::vector<const tinyxml2::XMLElement*> getChildNodes(
        const tinyxml2::XMLElement* parentNode,
        const char* tag
    ) const;
};

#endif  // XMLPARSER_H