#ifndef RSPOE_H
#define RSPOE_H

#include <rspoe_export.h>

enum PoeState
{
    StateDisabled,		//No power regardless of attached device.
    StateEnabled,		//Power always applied regardless of attached device.
    StateAuto,			//Normal PoE operation.
    StateError          //Error retreiving state. Use getLastError and getLastErrorString for more details.
};


class RsPoe {
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

extern "C" RSPOE_EXPORT RsPoe *createRsPoe();

#endif //RSPOE_H