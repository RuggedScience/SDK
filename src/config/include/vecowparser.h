#ifndef VECOWPARSER_H
#define VECOWPARSER_H

#include "xmlparser.h"
#include "tinyxml2.h"

class VecowXmlParser: public XmlParser
{
protected:
    void parseImpl(tinyxml2::XMLDocument &doc) override;
};


#endif // VECOWPARSER_H