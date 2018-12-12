#include "pd69104.h"
#include "../../utils/i801_smbus.hpp"

#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>

//Registers as described in the datasheet for the PD69104.
static const uint8_t kSataPwrReg = 0x10;	//Power Status register
static const uint8_t kOpmdReg = 0x12;		//Operating Mode register
static const uint8_t kDisenaReg = 0x13;     //Disconnect Sensing Enable register
static const uint8_t kDetenaReg = 0x14;     //Detection and Classification Enable register
static const uint8_t kPwrpbReg = 0x19;		//Power On/Off Pushbutton register
static const uint8_t kDevIdReg = 0x43;		//Device ID register. Should always read 0x44

static const uint8_t kShutdownMode = 0;
static const uint8_t kManualMode =	1;
static const uint8_t kSemiAutoMode = 2;
static const uint8_t kAutoMode = 3;

#include <iostream>

Pd69104::Pd69104(uint16_t bus, uint8_t dev) :
	AbstractPoeController(),
	m_busAddr(bus),
	m_devAddr(dev)
{
	int devId = getDeviceId();
	if (devId < 0)
		throw PoeControllerError(std::strerror(errno));
}

Pd69104::~Pd69104()
{

}

Poe::PoeState Pd69104::getPortState(uint8_t port)
{
	uint8_t mode = getPortMode(port);
	if (mode == kManualMode)
		return Poe::StateEnabled;
	else if (mode == kShutdownMode)
		return Poe::StateDisabled;
	else if (mode == kAutoMode)
		return Poe::StateAuto;
	else
		throw PoeControllerError("Unknown port mode found");
}

void Pd69104::setPortState(uint8_t port, Poe::PoeState state)
{
	switch (state)
	{
		case Poe::StateEnabled:
			setPortMode(port, kManualMode);
			setPortDetection(port, false);
			setPortClassification(port, false);
			setPortSensing(port, false);
			setPortEnabled(port, true);
			break;
		case Poe::StateDisabled:
			setPortMode(port, kShutdownMode);
			break;
		case Poe::StateAuto:
			setPortMode(port, kAutoMode);
			setPortDetection(port, true);
			setPortClassification(port, true);
			setPortSensing(port, true);
			break;
		case Poe::StateError:
			break;
	}
}

int Pd69104::getDeviceId()
{
	return smbusReadRegister(m_busAddr, m_devAddr, kDevIdReg);
}

void Pd69104::setPortEnabled(uint8_t port, bool enabled)
{
	uint8_t data = 0;
	if (enabled) data = (1 << port);
	else data = (1 << (port + 4));

	if (smbusWriteRegister(m_busAddr, m_devAddr, kPwrpbReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

uint8_t Pd69104::getPortMode(uint8_t port)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kOpmdReg);
	if (data < 0) 
		throw PoeControllerError(std::strerror(errno));

	//The mode is stored in two bits so lets shift it over until the two bits for our port are the LSBs.
	return ((data >> (port * 2)) & 0b11);
}

void Pd69104::setPortMode(uint8_t port, uint8_t mode)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kOpmdReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	data &= ~(0b11 << (port * 2));			//Make sure both bits for this port are low.
	data |= (mode << (port * 2));			//Then just OR the desired mode (shifted to the correct position) and we are good.

	if (smbusWriteRegister(m_busAddr, m_devAddr, kOpmdReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

bool Pd69104::getPortSensing(uint8_t port)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDisenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Sensing is enabled if the ports bit is set in the 4 MSBs or 4 LSBs so we check both.
	//We always only set the 4 LSBs but lets be safe in case someone else has been messing around in the registers.
	return (data & ((1 << (port + 4)) | (1 << port))) != 0;
}

void Pd69104::setPortSensing(uint8_t port, bool sense)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDisenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Bits 4-7 do the same thing as 0-3 on this chip (PD69104).
	//To avoid confusion, let's only work with bits 0-3 and always keeps bits 4-7 low.
	data &= 0x0F;
	if (sense) data |= (1 << port);
	else data &= ~(1 << port);

	if (smbusWriteRegister(m_busAddr, m_devAddr, kDisenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

bool Pd69104::getPortDetection(uint8_t port)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	return (data & (1 << port)) != 0;
}

void Pd69104::setPortDetection(uint8_t port, bool detect)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	if (detect) data |= (1 << port);
	else data &= ~(1 << port);

	if (smbusWriteRegister(m_busAddr, m_devAddr, kDetenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}


bool Pd69104::getPortClassification(uint8_t port)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Left shift by 4 since Classification is stored in the 4 MSBs
	return (data & (1 << (port + 4))) != 0;
}

void Pd69104::setPortClassification(uint8_t port, bool classify)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Classification and Detection are in the same register. 
	//4 MSBs are for Classification so we need to shift our bitmask.
	if (classify) data |= (1 << (port + 4));
	else data &= ~(1 << (port + 4));

	if (smbusWriteRegister(m_busAddr, m_devAddr, kDetenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}