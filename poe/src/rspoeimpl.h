#ifndef RSPOEIMPL_H
#define RSPOEIMPL_H

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "../include/rspoe.h"
#include "controllers/abstractpoecontroller.h"

typedef std::map<int, uint8_t> portmap_t;

class RsPoeImpl : public rs::RsPoe {
   public:
    RsPoeImpl();
    RsPoeImpl(AbstractPoeController *controller, portmap_t portMap);

    ~RsPoeImpl();

    void destroy() override;
    void setXmlFile(const char *fileName) override;

    std::vector<int> getPortList() const override;

    rs::PoeState getPortState(int port) override;
    void setPortState(int port, rs::PoeState state) override;

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
    portmap_t m_portMap;
    AbstractPoeController *mp_controller;
};

#endif  // RSPOEIMPL_H