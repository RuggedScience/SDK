#ifndef ABSTRACTDIOCONTROLLER_H
#define ABSTRACTDIOCONTROLLER_H

#include "dioconfig.h"
#include "rsdio_types.h"

#include <stdint.h>
#include <system_error>

enum PinMode
{
	ModeInput,
	ModeOutput
};

class AbstractDioController
{
public:
	virtual ~AbstractDioController() {}

    virtual void initPin(const DioPinConfig &config) = 0;
	virtual rs::PinDirection getPinMode(const DioPinConfig &config) = 0;
	virtual void setPinMode(const DioPinConfig &config, rs::PinDirection mode) = 0;

	virtual bool getPinState(const DioPinConfig &config) = 0;
	virtual void setPinState(const DioPinConfig &config, bool state) = 0;

	virtual void printRegs() = 0;
};

#endif