#ifndef I801_SMBUS_H
#define I801_SMBUS_H

#include <stdint.h>

uint8_t i801_smbus_read(uint16_t bus, uint8_t device);
void i801_smbus_write(uint16_t bus, uint8_t device, uint8_t command);

uint8_t i801_smbus_read_register(
    uint16_t bus, 
    uint8_t device, 
    uint8_t command
);

void i801_smbus_write_register(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t value
);

void i801_smbus_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t *block
);

void i801_smbus_write_block(
    uint16_t bus,
    uint8_t device,
    uint8_t *block,
    uint8_t size
);

void i801_i2c_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t *buf,
    uint8_t size
);

#endif  // I801_SMBUS_H