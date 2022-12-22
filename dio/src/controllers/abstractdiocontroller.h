#ifndef ABSTRACTDIOCONTROLLER_H
#define ABSTRACTDIOCONTROLLER_H

#include <stdint.h>
#include <system_error>  

enum PinMode
{
	ModeInput,
	ModeOutput
};

struct PinConfig
{
	uint8_t bitmask;
	uint8_t offset;
	bool invert;
	bool enablePullup;
	bool supportsInput;
    bool supportsOutput;

	PinConfig() 
		: bitmask(0)
		, offset(0)
		, invert(0)
		, enablePullup(false)
		, supportsInput(false)
		, supportsOutput(false)
	{}

	PinConfig(uint8_t bit, uint8_t gpio, bool inv, bool pullup, bool input, bool output) 
		: bitmask(1 << bit)
		, offset(gpio - 1)
		, invert(inv)
		, enablePullup(pullup)
		, supportsInput(input)
		, supportsOutput(output)
    {}
};


class AbstractDioController
{
public:
	virtual ~AbstractDioController() {}

    virtual void initPin(const PinConfig &config) = 0;
	virtual PinMode getPinMode(const PinConfig &config) = 0;
	virtual void setPinMode(const PinConfig &config, PinMode mode) = 0;

	virtual bool getPinState(const PinConfig &config) = 0;
	virtual void setPinState(const PinConfig &config, bool state) = 0;

	virtual void printRegs() = 0;
};

#endif