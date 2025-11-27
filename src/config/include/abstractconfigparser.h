#ifndef ABSTRACTCONFIGPARSER_H
#define ABSTRACTCONFIGPARSER_H

#include <map>
#include <string>
#include <vector>

#include "dioconfig.h"
#include "poeconfig.h"

class AbstractConfigParser {
   public:
    void parse(const char* fileName);
    virtual std::vector<DioControllerConfig> getDioControllerConfigs() const;
    virtual std::vector<PoeControllerConfig> getPoeControllerConfigs() const;

   protected:
    virtual void parseImpl(const char* fileName) = 0;

    void addDioConfig(DioControllerConfig config);
    void addPoeConfig(PoeControllerConfig config);

   private:
    std::vector<DioControllerConfig> m_dioControllerConfigs;
    std::vector<PoeControllerConfig> m_poeControllerConfigs;
};
#endif  // ABSTRACTCONFIGPARSER_H