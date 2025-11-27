#include "../include/smbus.h"
#include "i801_smbus.h"

/**
 * For now this just wraps the i801 SMBus implementation.
 */

uint8_t portio_smbus_read(uint16_t bus, uint8_t device)
{
    return i801_smbus_read(bus, device);
}

void portio_smbus_write(uint16_t bus, uint8_t device, uint8_t command)
{
    return i801_smbus_write(bus, device, command);
}

uint8_t portio_smbus_read_register(
    uint16_t bus, 
    uint8_t device, 
    uint8_t command
)
{
    return i801_smbus_read_register(bus, device, command);
}

void portio_smbus_write_register(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t value
)
{
    return i801_smbus_write_register(bus, device, command, value);
}

void portio_smbus_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t* block
)
{
    return i801_smbus_read_block(bus, device, command, block);
}


void portio_smbus_write_block(
    uint16_t bus,
    uint8_t device,
    uint8_t* block,
    uint8_t size
)
{
    return i801_smbus_write_block(bus, device, block, size);
}

void portio_i2c_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t* buf,
    uint8_t size
)
{
    return i801_i2c_read_block(bus, device, command, buf, size);
}