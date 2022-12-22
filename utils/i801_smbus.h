#ifndef SMBUS_H
#define SMBUS_H

#include <stdint.h>

uint8_t smbusReadRegister(uint16_t bus, uint8_t dev, uint8_t reg);
void smbusWriteRegister(uint16_t bus, uint8_t dev, uint8_t reg, uint8_t val);

uint8_t smbusReadByte(uint16_t bus, uint8_t dev);
void smbusWriteByte(uint16_t bus, uint8_t dev, uint8_t val);

void smbusI2CRead(uint16_t bus, uint8_t dev, uint8_t cmd, uint8_t *buf, size_t size);

#endif //SMBUS_H