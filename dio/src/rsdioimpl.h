#ifndef RSDIOIMPL_H
#define RSDIOIMPL_H

#include "rsdio.h"
#include "controllers/abstractdiocontroller.h"

#include <string>

typedef std::map<int, PinConfig> pinconfigmap_t;
typedef std::map<int, pinconfigmap_t> dioconfigmap_t;

class RsDioImpl : public rs::RsDio
{
public:
    RsDioImpl();
    RsDioImpl(AbstractDioController *controller, dioconfigmap_t dioMap);
    ~RsDioImpl();

    void destroy() override;
    bool setXmlFile(const char *fileName, bool debug=false) override;

    rs::diomap_t getPinList() const override;

    int canSetOutputMode(int dio) override;
    bool setOutputMode(int dio, rs::OutputMode mode) override;

    int digitalRead(int dio, int pin) override;
    bool digitalWrite(int dio, int pin, bool state) override;

    std::error_code getLastError() const;
    std::string getLastErrorString() const;

private:
    std::error_code m_lastError;
    std::string m_lastErrorString;
    AbstractDioController *mp_controller;

    dioconfigmap_t m_dioMap;
};

#endif //RSDIOIMPL_H