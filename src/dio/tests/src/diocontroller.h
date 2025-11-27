#include "controllers/abstractdiocontroller.h"

#include <map>

struct PinStatus
{
    bool state;
    rs::PinDirection mode;
    DioPinConfig config;
};

class TestDioController : public AbstractDioController
{
public:
    TestDioController() {}

    void initPin(const DioPinConfig &config) override final
    {
        uint16_t id = idFromConfig(config);
        PinStatus status;
        status.state = 0;
        status.config = config;
        if (config.supportsInput)
            status.mode = rs::PinDirection::Input;
        else
            status.mode = rs::PinDirection::Output;
        
        m_pins[id] = status;
    }

    rs::PinDirection getPinMode(const DioPinConfig &config) override final
    {
        return getOrThrow(config).mode;
    }

    void setPinMode(const DioPinConfig &config, rs::PinDirection mode) override final
    {
        getOrThrow(config).mode = mode;
    }

    bool getPinState(const DioPinConfig &config) override final
    {
        return getOrThrow(config).state;
    }

    void setPinState(const DioPinConfig &config, bool state) override final
    {
        getOrThrow(config).state = state;
    }

    void printRegs() override final {}

private:
    uint16_t idFromConfig(const DioPinConfig &config) const
    {
        return config.bitmask << 8 | config.offset;
    }

    PinStatus &getOrThrow(const DioPinConfig &config)
    {
        uint16_t id = idFromConfig(config);
        auto it = m_pins.find(id);
        if (it == m_pins.end())
        {
            throw std::system_error(
                std::make_error_code(std::errc::invalid_argument), 
                "Pin never initialized"
            );
        }
        return it->second;
    }

    std::map<uint16_t, PinStatus> m_pins;
};