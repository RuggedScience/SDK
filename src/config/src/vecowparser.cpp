#include "vecowparser.h"
#include "rserrors.h"

using namespace tinyxml2;

void VecowXmlParser::parseImpl(XMLDocument &doc)
{
    const XMLElement* machineNode = doc.FirstChildElement("machine");
    if (!machineNode) 
        throw rs::RsException(rs::RsErrorCode::ConfigParseError, "Invalid config format");

    for (auto ioportNode : getChildNodes(machineNode, "ioport")) {
        
    }
}
