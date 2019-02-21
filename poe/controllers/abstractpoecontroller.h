#ifndef ABSTRACTPOECONTROLLER_H
#define ABSTRACTPOECONTROLLER_H

#include "../rspoe.h"

#include <exception>
#include <stdint.h>

class PoeControllerError: public std::exception
{
public:
    PoeControllerError(const char *what) : 
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

class AbstractPoeController
{
public:
	virtual ~AbstractPoeController() {}

	virtual PoeState getPortState(uint8_t port) = 0;
	virtual void setPortState(uint8_t port, PoeState state) = 0;

    virtual float getPortVoltage(uint8_t port) { throw PoeControllerError("Function not supported"); } // Should always return volts
	virtual float getPortCurrent(uint8_t port) { throw PoeControllerError("Function not supported"); } // Should always return amps
	virtual float getPortPower(uint8_t port)
    {
        return getPortVoltage(port) * getPortCurrent(port);
    };

    virtual int getBudgetConsumed()     { throw PoeControllerError("Function not supported"); }
    virtual int getBudgetAvailable()    { throw PoeControllerError("Function not supported"); }
    virtual int getBudgetTotal()        { throw PoeControllerError("Function not supported"); }
};

#endif