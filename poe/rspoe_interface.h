#ifndef RSPOE_INTERFACE_H
#define RSPOE_INTERFACE_H

#include "rspoe_global.h"

class RsPoeInterface {
public:
    virtual void destroy() = 0;
    virtual bool setXmlFile(const char *fileName) = 0;
    virtual PoeState getPortState(int port) = 0;
    virtual int setPortState(int port, PoeState state) = 0;
    virtual float getPortVoltage(int port) = 0;
    virtual float getPortCurrent(int port) = 0;
    virtual float getPortPower(int port) = 0;
    virtual int getBudgetConsumed() = 0;
    virtual int getBudgetAvailable() = 0;
    virtual int getBudgetTotal() = 0;
    virtual const char *getLastError() = 0;
};

#endif //RSPOE_INTERFACE_H