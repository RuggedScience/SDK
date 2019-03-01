#ifndef RSDIO_H
#define RSDIO_H

#include <string>
#include <rsdio_export.h>

enum OutputMode
{
    ModeError = 0,
    ModePnp = -1,
    ModeNpn = -2    
};

class RsDio {
public:
    virtual void destroy() = 0;
    virtual bool setXmlFile(const char *fileName) = 0;
    virtual int digitalRead(int dio, int pin) = 0;
    virtual int digitalWrite(int dio, int pin, bool state) = 0;
    virtual int setOutputMode(int dio, OutputMode mode) = 0;
    virtual std::string getLastError() = 0;
    virtual std::string version() = 0;
};

extern "C" RSDIO_EXPORT RsDio *createRsDio();

#endif //RSDIO_H