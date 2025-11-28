#ifndef RSPOEIMPL_H
#define RSPOEIMPL_H

#include "rspoe.h"
#include "poeconfig.h"
#include "controllers/abstractpoecontroller.h"

#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class RsPoeImpl : public rs::RsPoe {
   public:
    RsPoeImpl();
    RsPoeImpl(std::unique_ptr<AbstractPoeController> controller, PoeControllerConfig config);

    void destroy() override;
    void init(const char *configFile) override;

    std::vector<int> getPortList() const override;

    rs::PoeState getPortState(int port) override;
    void setPortState(int port, rs::PoeState state) override;

    float getPortVoltage(int port) override;
    float getPortCurrent(int port) override;
    float getPortPower(int port) override;

    int getBudgetConsumed() override;
    int getBudgetAvailable() override;
    int getBudgetTotal() override;

   private:
    PoeControllerConfig m_config;
    std::unique_ptr<AbstractPoeController> m_controller;

    void ensureInitialized() const;
    const PoePortConfig & getPortConfig(int port) const;
};

#endif  // RSPOEIMPL_H