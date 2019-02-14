#ifndef RSDIO_H
#define RSDIO_H

#include <rsdio_export.h>
#include "rsdio_interface.h"
#include "controllers/abstractdiocontroller.h"

#include <map>
#include <string>

class RsDio : public RsDioInterface
{
public:
    RsDio();
    ~RsDio();

    void destroy() override;
    bool setXmlFile(const char *fileName) override;
    int digitalRead(int dio, int pin) override;
    int digitalWrite(int dio, int pin, bool state) override;
    int setOutputMode(int dio, OutputMode mode) override;
    const char *getLastError() override;

private:
    std::string m_lastError;
    AbstractDioController *mp_controller;

    typedef std::map<int, PinInfo> pinmap_t;
    std::map<int, pinmap_t> m_dioMap;
};

extern "C" RSDIO_EXPORT RsDioInterface *createRsDio();

#endif //RSDIO_H