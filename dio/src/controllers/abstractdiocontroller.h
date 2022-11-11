#ifndef ABSTRACTDIOCONTROLLER_H
#define ABSTRACTDIOCONTROLLER_H

#include <rsdio.h>

#include <stdint.h>
#include <system_error>  

enum PinMode
{
	ModeInput,
	ModeOutput
};

struct PinConfig : public rs::PinInfo
{
	uint8_t bitmask;
	uint8_t offset;
	bool invert;
	bool enablePullup;

	PinConfig() 
		: PinInfo()
		, bitmask(0)
		, offset(0)
		, invert(0)
		, enablePullup(false)
	{}

	PinConfig(uint8_t bit, uint8_t gpio, bool inv, bool pullup, bool input, bool output) 
		: PinInfo(input, output)
		, bitmask(1 << bit)
		,  offset(gpio - 1)
		, invert(inv)
		, enablePullup(pullup)
    {}
};


class AbstractDioController
{
public:
	virtual ~AbstractDioController() {}

    virtual void initPin(PinConfig) = 0;
	virtual PinMode getPinMode(PinConfig info) = 0;
	virtual void setPinMode(PinConfig info, PinMode mode) = 0;

	virtual bool getPinState(PinConfig info) = 0;
	virtual void setPinState(PinConfig info, bool state) = 0;

	virtual void printRegs() = 0;
};

#endif