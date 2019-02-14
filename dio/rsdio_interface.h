#ifndef RSDIO_INTERFACE_H
#define RSDIO_INTERFACE_H

#include "rsdio_global.h"

class RsDioInterface {
public:
    virtual void destroy() = 0;
    virtual bool setXmlFile(const char *fileName) = 0;
    virtual int digitalRead(int dio, int pin) = 0;
    virtual int digitalWrite(int dio, int pin, bool state) = 0;
    virtual int setOutputMode(int dio, OutputMode mode) = 0;
    virtual const char *getLastError() = 0;
};

#endif //RSDIO_INTERFACE_H