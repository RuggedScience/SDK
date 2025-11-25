#ifndef RSDIO_H
#define RSDIO_H

#include <map>
#include <string>
#include <system_error>

#ifdef NO_EXPORT
#define RSDIO_EXPORT
#else
#include "rsdio_export.h"
#endif

namespace rs {

struct PinInfo {
    bool supportsInput;
    bool supportsOutput;

    PinInfo() : supportsInput(false), supportsOutput(false) {}

    PinInfo(bool input, bool output)
        : supportsInput(input), supportsOutput(output)
    {
    }
};

typedef std::map<int, PinInfo> pinmap_t;
typedef std::map<int, pinmap_t> diomap_t;

enum class OutputMode { Source = -1, Sink = -2 };

enum class PinDirection { Input, Output };

class RsDio {
   public:
    virtual ~RsDio() {}

    virtual void destroy() = 0;
    virtual void setXmlFile(const char *fileName, bool debug = false) = 0;

    virtual diomap_t getPinList() const = 0;

    virtual bool canSetOutputMode(int dio) = 0;
    virtual void setOutputMode(int dio, OutputMode mode) = 0;
    virtual OutputMode getOutputMode(int dio) = 0;

    virtual bool digitalRead(int dio, int pin) = 0;
    virtual void digitalWrite(int dio, int pin, bool state) = 0;

    virtual void setPinDirection(int dio, int pin, PinDirection dir) = 0;
    virtual PinDirection getPinDirection(int dio, int pin) = 0;

    virtual std::map<int, bool> readAll(int dio) = 0;

    virtual std::error_code getLastError() const = 0;
    virtual std::string getLastErrorString() const = 0;
};

extern "C" RSDIO_EXPORT RsDio *createRsDio();
extern "C" RSDIO_EXPORT const char *rsDioVersion();

}  // namespace rs

#endif  // RSDIO_H