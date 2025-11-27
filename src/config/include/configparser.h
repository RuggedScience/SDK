#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include "abstractconfigparser.h"

#include <memory>

class ConfigParser : public AbstractConfigParser
{   
protected:
    void parseImpl(const char* fileName);
};

#endif // CONFIGPARSER_H