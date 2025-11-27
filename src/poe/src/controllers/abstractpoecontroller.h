#ifndef ABSTRACTPOECONTROLLER_H
#define ABSTRACTPOECONTROLLER_H

#include "rspoe.h"
#include "rserrors.h"

#include <stdint.h>


class AbstractPoeController
{
public:
	virtual ~AbstractPoeController() {}

	virtual rs::PoeState getPortState(uint8_t port) = 0;
	virtual void setPortState(uint8_t port, rs::PoeState state) = 0;

    virtual float getPortVoltage(uint8_t port) { throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Querying port voltage not supported"); }
	virtual float getPortCurrent(uint8_t port) { throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Querying port current not supported"); }
	virtual float getPortPower(uint8_t port)
    {
        return getPortVoltage(port) * getPortCurrent(port);
    };

    virtual int getBudgetConsumed()     { throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Querying budget consumed");  }
    virtual int getBudgetAvailable()    { throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Querying budget available not supported");  }
    virtual int getBudgetTotal()        { throw rs::RsException(rs::RsErrorCode::UnsupportedFunction, "Querying budget total not supported");  }
};

#endif