#ifndef RSPOEIMPL_H
#define RSPOEIMPL_H

#include "rspoe.h"
#include "controllers/abstractpoecontroller.h"

#include <map>
#include <string>
#include <stdint.h>

class RsPoeImpl : public RsPoe
{
public:
    RsPoeImpl();
    ~RsPoeImpl();

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
    std::string getLastError() override;

private:
    std::string m_lastError;
    AbstractPoeController *mp_controller;
    std::map<int, uint8_t> m_portMap;
};

#endif //RSPOEIMPL_H