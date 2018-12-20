#ifndef ABSTRACTPOECONTROLLER_H
#define ABSTRACTPOECONTROLLER_H

#include "../poe_global.h"

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
};

#endif