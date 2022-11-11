#ifndef ABSTRACTPOECONTROLLER_H
#define ABSTRACTPOECONTROLLER_H

#include "rssdk_errors.hpp"

#include <rspoe.h>
#include <stdint.h>


class AbstractPoeController
{
public:
	virtual ~AbstractPoeController() {}

	virtual rs::PoeState getPortState(uint8_t port) = 0;
	virtual void setPortState(uint8_t port, rs::PoeState state) = 0;

    virtual float getPortVoltage(uint8_t port) { throw std::system_error(rs::RsSdkError::FunctionNotSupported); }
	virtual float getPortCurrent(uint8_t port) { throw std::system_error(rs::RsSdkError::FunctionNotSupported); }
	virtual float getPortPower(uint8_t port)
    {
        return getPortVoltage(port) * getPortCurrent(port);
    };

    virtual int getBudgetConsumed()     { throw std::system_error(rs::RsSdkError::FunctionNotSupported); }
    virtual int getBudgetAvailable()    { throw std::system_error(rs::RsSdkError::FunctionNotSupported); }
    virtual int getBudgetTotal()        { throw std::system_error(rs::RsSdkError::FunctionNotSupported); }
};

#endif