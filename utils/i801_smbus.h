#ifndef SMBUS_H
#define SMBUS_H

#include <stdint.h>

uint8_t smbus_read(uint16_t bus, uint8_t device);
void smbus_write(uint16_t bus, uint8_t device, uint8_t command);

uint8_t smbus_read_register(uint16_t bus, uint8_t device, uint8_t command);
void smbus_write_register(uint16_t bus, uint8_t device, uint8_t command, uint8_t value);

void i2c_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t *buf,
    size_t size
);

#endif  // SMBUS_H