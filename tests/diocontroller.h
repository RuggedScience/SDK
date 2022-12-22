#include <map>

#include "../dio/src/controllers/abstractdiocontroller.h"

struct PinStatus
{
    bool state;
    PinMode mode;
    PinConfig config;
};

class TestDioController : public AbstractDioController
{
public:
    TestDioController() {}

    void initPin(const PinConfig &config) override final
    {
        uint16_t id = idFromConfig(config);
        PinStatus status;
        status.state = 0;
        status.config = config;
        if (config.supportsInput)
            status.mode = PinMode::ModeInput;
        else
            status.mode = PinMode::ModeOutput;
        
        m_pins[id] = status;
    }

    PinMode getPinMode(const PinConfig &config) override final
    {
        return getOrThrow(config).mode;
    }

    void setPinMode(const PinConfig &config, PinMode mode) override final
    {
        getOrThrow(config).mode = mode;
    }

    bool getPinState(const PinConfig &config) override final
    {
        return getOrThrow(config).state;
    }

    void setPinState(const PinConfig &config, bool state) override final
    {
        getOrThrow(config).state = state;
    }

    void printRegs() override final {}

private:
    uint16_t idFromConfig(const PinConfig &config) const
    {
        return config.bitmask << 8 | config.offset;
    }

    PinStatus &getOrThrow(const PinConfig &config)
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