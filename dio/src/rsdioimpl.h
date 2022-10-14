#ifndef RSDIOIMPL_H
#define RSDIOIMPL_H

#include "rsdio.h"
#include "controllers/abstractdiocontroller.h"

#include <string>

class RsDioImpl : public RsDio
{
public:
    RsDioImpl();
    ~RsDioImpl();

    void destroy() override;
    bool setXmlFile(const char *fileName, bool debug=false) override;

    diomap_t getPinList() const override;

    int canSetOutputMode(int dio) override;
    int setOutputMode(int dio, OutputMode mode) override;

    int digitalRead(int dio, int pin) override;
    int digitalWrite(int dio, int pin, bool state) override;
    
    std::string getLastError() override;
    std::string version() const override { return std::string(RSDIO_VERSION_STRING); }

private:
    std::string m_lastError;
    AbstractDioController *mp_controller;

    typedef std::map<int, PinConfig> pinconfigmap_t;
    typedef std::map<int, pinconfigmap_t> dioconfigmap_t;
    dioconfigmap_t m_dioMap;
};

#endif //RSDIOIMPL_H