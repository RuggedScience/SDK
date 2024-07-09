#ifndef RSDIOIMPL_H
#define RSDIOIMPL_H

#include <string>

#include "../include/rsdio.h"
#include "controllers/abstractdiocontroller.h"

typedef std::map<int, PinConfig> pinconfigmap_t;
typedef std::map<int, pinconfigmap_t> dioconfigmap_t;

class RsDioImpl : public rs::RsDio {
   public:
    RsDioImpl();
    RsDioImpl(AbstractDioController *controller, dioconfigmap_t dioMap);
    ~RsDioImpl();

    void destroy() override;
    void setXmlFile(const char *fileName, bool debug = false) override;

    rs::diomap_t getPinList() const override;

    bool canSetOutputMode(int dio) override;
    void setOutputMode(int dio, rs::OutputMode mode) override;
    rs::OutputMode getOutputMode(int dio) override;

    bool digitalRead(int dio, int pin) override;
    void digitalWrite(int dio, int pin, bool state) override;

    void setPinDirection(int dio, int pin, rs::PinDirection dir) override;
    rs::PinDirection getPinDirection(int dio, int pin) override;

    std::map<int, bool> readAll(int dio) override;

    std::error_code getLastError() const;
    std::string getLastErrorString() const;

   private:
    std::error_code m_lastError;
    std::string m_lastErrorString;
    dioconfigmap_t m_dioMap;
    AbstractDioController *mp_controller;
};

#endif  // RSDIOIMPL_H