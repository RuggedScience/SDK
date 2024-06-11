#ifndef RSPOE_H
#define RSPOE_H

#include <string>
#include <system_error>
#include <vector>

#ifdef NO_EXPORT
#define RSPOE_EXPORT
#else
#include "rspoe_export.h"
#endif

namespace rs {

enum class PoeState {
    Disabled,  // No power regardless of attached device.
    Enabled,   // Power always applied regardless of attached device.
    Auto,      // Normal PoE operation.
    Error  // Error retreiving state. Use getLastError and getLastErrorString
           // for more details.
};

class RsPoe {
   public:
    virtual ~RsPoe(){};

    virtual void destroy() = 0;
    virtual void setXmlFile(const char *fileName) = 0;

    virtual std::vector<int> getPortList() const = 0;

    virtual PoeState getPortState(int port) = 0;
    virtual void setPortState(int port, PoeState state) = 0;

    virtual float getPortVoltage(int port) = 0;
    virtual float getPortCurrent(int port) = 0;
    virtual float getPortPower(int port) = 0;

    virtual int getBudgetConsumed() = 0;
    virtual int getBudgetAvailable() = 0;
    virtual int getBudgetTotal() = 0;

    virtual std::error_code getLastError() const = 0;
    virtual std::string getLastErrorString() const = 0;
};

extern "C" RSPOE_EXPORT RsPoe *createRsPoe();
extern "C" RSPOE_EXPORT const char *rsPoeVersion();

}  // namespace rs

#endif  // RSPOE_H