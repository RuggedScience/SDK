#ifndef POECONFIG_H
#define POECONFIG_H

#include <stdint.h>
#include <string>
#include <vector>

struct PoePortConfig
{
    int id;
    uint8_t offset;
};

struct PoeControllerConfig
{
    std::string controllerId;
    uint16_t busAddr;
    uint8_t chipAddr;

    std::vector<PoePortConfig> ports;
};

#endif // POECONFIG_H