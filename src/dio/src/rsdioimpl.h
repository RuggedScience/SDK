#ifndef RSDIOIMPL_H
#define RSDIOIMPL_H

#include <string>
#include <memory>

#include "rsdio.h"
#include "controllers/abstractdiocontroller.h"

class RsDioImpl : public rs::RsDio {
   public:
    RsDioImpl();
    RsDioImpl(std::unique_ptr<AbstractDioController> controller, DioControllerConfig config);

    void destroy() override;
    void init(const char *configFile) override;

    rs::diomap_t getPinList() const override;

    bool canSetOutputMode(int dio) override;
    void setOutputMode(int dio, rs::OutputMode mode) override;
    rs::OutputMode getOutputMode(int dio) override;

    bool digitalRead(int dio, int pin) override;
    void digitalWrite(int dio, int pin, bool state) override;

    void setPinDirection(int dio, int pin, rs::PinDirection dir) override;
    rs::PinDirection getPinDirection(int dio, int pin) override;

    std::map<int, bool> readAll(int dio) override;

   private:
    DioControllerConfig m_config;
    std::unique_ptr<AbstractDioController> m_controller;

    void ensureInitialized() const;
    const DioConnectorConfig & getConnectorConfig(int dio) const;
    const DioPinConfig & getPinConfig(int dio, int pin) const;
    const DioPinConfig & getPinConfig(const DioConnectorConfig &dio, int pin) const;
};

#endif  // RSDIOIMPL_H