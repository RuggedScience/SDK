#include "abstractconfigparser.h"

void AbstractConfigParser::parse(const char* fileName)
{
    m_dioControllerConfigs.clear();
    m_poeControllerConfigs.clear();
    parseImpl(fileName);
}

std::vector<DioControllerConfig> AbstractConfigParser::getDioControllerConfigs() const
{
    return m_dioControllerConfigs;
}

std::vector<PoeControllerConfig> AbstractConfigParser::getPoeControllerConfigs() const
{
    return m_poeControllerConfigs;
}

void AbstractConfigParser::addDioConfig(DioControllerConfig config)
{
    m_dioControllerConfigs.push_back(config);
}

void AbstractConfigParser::addPoeConfig(PoeControllerConfig config)
{
    m_poeControllerConfigs.push_back(config);
}