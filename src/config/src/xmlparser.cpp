#include "xmlparser.h"
#include "rserrors.h"

using namespace tinyxml2;

void XmlParser::parseImpl(const char* fileName)
{
    XMLDocument doc;
    if (doc.LoadFile(fileName) != XML_SUCCESS) {
        switch (doc.ErrorID()) {
            case XML_ERROR_FILE_NOT_FOUND:
                throw rs::RsException(rs::RsErrorCode::FileNotFound, std::string(fileName) + " not found");
            case XML_ERROR_FILE_COULD_NOT_BE_OPENED:
                throw rs::RsException(rs::RsErrorCode::ConfigParseError, std::string(fileName) + " could not be opened");
            case XML_ERROR_FILE_READ_ERROR:
                throw rs::RsException(rs::RsErrorCode::ConfigParseError, std::string(fileName) + "is not a valid config file");
            default:
                throw rs::RsException(rs::RsErrorCode::ConfigParseError, "Unknown error occured while parsing " + std::string(fileName));
        }
    }
    parseImpl(doc);
}

std::vector<const XMLElement *> XmlParser::getChildNodes(const XMLElement *parent, const char *tag) const
{
    std::vector<const XMLElement *> children;
    const XMLElement *child = parent->FirstChildElement(tag);
    while (child)
    {
        children.push_back(child);
        child = child->NextSiblingElement(tag);
    }
    return children;
}