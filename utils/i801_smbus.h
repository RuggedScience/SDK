#ifndef SMBUS_H
#define SMBUS_H

#include <stdint.h>

int smbusReadRegister(uint16_t bus, uint8_t dev, uint8_t reg);
int smbusWriteRegister(uint16_t bus, uint8_t dev, uint8_t reg, uint8_t val);

int smbusReadByte(uint16_t bus, uint8_t dev);
int smbusWriteByte(uint16_t bus, uint8_t dev, uint8_t val);

int smbusI2CRead(uint16_t bus, uint8_t dev, uint8_t cmd, uint8_t *buf, size_t size);

#endif //SMBUS_H