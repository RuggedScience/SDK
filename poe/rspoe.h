#ifndef POE_H
#define POE_H

#include <rspoe_export.h>
#include "rspoe_interface.h"
#include "controllers/abstractpoecontroller.h"

#include <map>
#include <string>
#include <stdint.h>

class RsPoe : public RsPoeInterface
{
public:
    RsPoe();
    ~RsPoe();

    void destroy() override;
    bool setXmlFile(const char *fileName) override;
    PoeState getPortState(int port) override;
    int setPortState(int port, PoeState state) override;
    float getPortVoltage(int port) override;
    float getPortCurrent(int port) override;
    float getPortPower(int port) override;
    int getBudgetConsumed() override;
    int getBudgetAvailable() override;
    int getBudgetTotal() override;
    const char *getLastError() override;

private:
    std::string m_lastError;
    AbstractPoeController *mp_controller;
    std::map<int, uint8_t> m_portMap;
};

extern "C" RSPOE_EXPORT RsPoeInterface * __cdecl createRsPoe();


#endif