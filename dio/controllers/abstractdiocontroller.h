#ifndef ABSTRACTDIOCONTROLLER_H
#define ABSTRACTDIOCONTROLLER_H

#include "../rsdio.h"

#include <stdint.h>
#include <exception>

enum PinMode
{
	ModeInput,
	ModeOutput
};

struct PinInfo
{
	uint8_t bitmask;
	uint8_t offset;
	bool invert;
	bool enablePullup;
	bool supportsInput;
	bool supportsOutput;

	PinInfo() :
		bitmask(0),
		offset(0),
		invert(0),
		enablePullup(false),
		supportsInput(false),
		supportsOutput(false)
	{}

	PinInfo(uint8_t bit, uint8_t gpio, bool inv, bool pullup, bool input, bool output) :
		bitmask(1 << bit),
		offset(gpio - 1),
		invert(inv),
		enablePullup(pullup),
		supportsInput(input),
		supportsOutput(output)
    {}
};

class DioControllerError: public std::exception
{
public:
    DioControllerError(const char *what) : 
		m_what(what)
    {
    }

    virtual const char *what() const throw()
    {
        return m_what;
    }

private:  
    const char *m_what;
};

class AbstractDioController
{
public:
	virtual ~AbstractDioController() {}

    virtual void initPin(PinInfo) = 0;
	virtual PinMode getPinMode(PinInfo info) = 0;
	virtual void setPinMode(PinInfo info, PinMode mode) = 0;

	virtual bool getPinState(PinInfo info) = 0;
	virtual void setPinState(PinInfo info, bool state) = 0;
};

#endif