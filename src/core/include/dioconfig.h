#ifndef DIOCONFIG_H
#define DIOCONFIG_H

#include <stdint.h>

#include <string>
#include <vector>

#define SINK_PIN_ID -2
#define SOURCE_PIN_ID -1

struct RegisterConfig {
    uint8_t addr;
    uint8_t ldn;
    uint8_t onBits;
    uint8_t offBits;
};

struct DioPinConfig {
    int id;
    uint8_t bitmask;
    uint8_t offset;
    bool invert;
    bool enablePullup;
    bool supportsInput;
    bool supportsOutput;

    DioPinConfig()
        : id(0),
          bitmask(0),
          offset(0),
          invert(0),
          enablePullup(false),
          supportsInput(false),
          supportsOutput(false)
    {
    }

    DioPinConfig(
        int id,
        uint8_t bit,
        uint8_t gpio,
        bool inv,
        bool pullup,
        bool input,
        bool output
    )
        : id(id),
          bitmask(1 << bit),
          offset(gpio - 1),
          invert(inv),
          enablePullup(pullup),
          supportsInput(input),
          supportsOutput(output)
    {
    }
};

struct DioConnectorConfig {
    int connectorId;
    DioPinConfig sinkPin;
    DioPinConfig sourcePin;
    std::vector<DioPinConfig> pins;

    bool supportsOutputConfig() const
    {
        // Config files use -2 to designate the sink pin
        // and -1 to designate the source pin.
        return sinkPin.id == SINK_PIN_ID && sourcePin.id == SOURCE_PIN_ID;
    }
};

struct DioControllerConfig {
    std::string controllerId;
    std::vector<DioConnectorConfig> connectors;
    std::vector<RegisterConfig> registers;
};

#endif  // DIOCONFIG_H