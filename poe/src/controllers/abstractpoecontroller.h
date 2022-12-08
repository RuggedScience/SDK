#ifndef ABSTRACTPOECONTROLLER_H
#define ABSTRACTPOECONTROLLER_H

#include "../../include/rspoe.h"

#include <stdint.h>


class AbstractPoeController
{
public:
	virtual ~AbstractPoeController() {}

	virtual rs::PoeState getPortState(uint8_t port) = 0;
	virtual void setPortState(uint8_t port, rs::PoeState state) = 0;

    virtual float getPortVoltage(uint8_t port) { throw std::system_error(std::make_error_code(std::errc::function_not_supported)); }
	virtual float getPortCurrent(uint8_t port) { throw std::system_error(std::make_error_code(std::errc::function_not_supported)); }
	virtual float getPortPower(uint8_t port)
    {
        return getPortVoltage(port) * getPortCurrent(port);
    };

    virtual int getBudgetConsumed()     { throw std::system_error(std::make_error_code(std::errc::function_not_supported)); }
    virtual int getBudgetAvailable()    { throw std::system_error(std::make_error_code(std::errc::function_not_supported)); }
    virtual int getBudgetTotal()        { throw std::system_error(std::make_error_code(std::errc::function_not_supported)); }
};

#endif