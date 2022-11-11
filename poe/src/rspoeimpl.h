#ifndef RSPOEIMPL_H
#define RSPOEIMPL_H

#include "rspoe.h"
#include "controllers/abstractpoecontroller.h"

#include <map>
#include <string>
#include <stdint.h>

class RsPoeImpl : public rs::RsPoe
{
public:
    RsPoeImpl();
    ~RsPoeImpl();

    void destroy() override;
    bool setXmlFile(const char *fileName) override;

    rs::PoeState getPortState(int port) override;
    int setPortState(int port, rs::PoeState state) override;

    float getPortVoltage(int port) override;
    float getPortCurrent(int port) override;
    float getPortPower(int port) override;

    int getBudgetConsumed() override;
    int getBudgetAvailable() override;
    int getBudgetTotal() override;

    std::error_code getLastError() const override;
    std::string getLastErrorString() const override;

private:
    std::error_code m_lastError;
    std::string m_lastErrorString;
    AbstractPoeController *mp_controller;
    std::map<int, uint8_t> m_portMap;
};

#endif //RSPOEIMPL_H