#ifndef ABSTRACTDIOCONTROLLER_H
#define ABSTRACTDIOCONTROLLER_H

#include "../dio.h"

#include <stdint.h>
#include <exception>

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

    virtual void initPin(Dio::PinInfo) = 0;
	virtual Dio::PinMode getPinMode(Dio::PinInfo info) = 0;
	virtual void setPinMode(Dio::PinInfo info, Dio::PinMode mode) = 0;

	virtual bool getPinState(Dio::PinInfo info) = 0;
	virtual void setPinState(Dio::PinInfo info, bool state) = 0;
};

#endif