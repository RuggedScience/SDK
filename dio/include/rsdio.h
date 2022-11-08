#ifndef RSDIO_H
#define RSDIO_H

#include <string>
#include <map>

#include "rsdio_export.h"

struct PinInfo
{
	bool supportsInput;
	bool supportsOutput;

	PinInfo() 
        : supportsInput(false)
		, supportsOutput(false)
	{}

	PinInfo(bool input, bool output) 
        : supportsInput(input)
		, supportsOutput(output)
    {}
};

typedef std::map<int, PinInfo> pinmap_t;
typedef std::map<int, pinmap_t> diomap_t;

enum OutputMode
{
    ModeError = 0,
    ModePnp = -1,
    ModeNpn = -2    
};

class RsDio {
public:
    virtual ~RsDio(){};

    virtual void destroy() = 0;
    virtual bool setXmlFile(const char *fileName, bool debug=false) = 0;

    virtual diomap_t getPinList() const = 0;

    virtual int canSetOutputMode(int dio) = 0;
    virtual int setOutputMode(int dio, OutputMode mode) = 0;

    virtual int digitalRead(int dio, int pin) = 0;
    virtual int digitalWrite(int dio, int pin, bool state) = 0;

    virtual std::string getLastError() = 0;
    virtual std::string version() const = 0;
};

extern "C" RSDIO_EXPORT RsDio *createRsDio();

#endif //RSDIO_H