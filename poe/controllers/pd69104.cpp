#include "pd69104.h"

#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <linux/i2c-dev.h>
#include <unistd.h>

static const uint8_t kBus = 5;
static const uint8_t kDevAddr = 0x20;		//SMBus address of the PD69104B1

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

Pd69104::Pd69104() :
	AbstractPoeController(),
	m_smbusfd(0)
{
	m_smbusfd = open("/dev/i2c-5", O_RDWR);
	if (m_smbusfd < 0)
		throw PoeControllerError(std::strerror(errno));

	if (ioctl(m_smbusfd, I2C_SLAVE, kDevAddr) < 0)
	{
		close(m_smbusfd);
		throw PoeControllerError(std::strerror(errno));
	}

	int devId = getDeviceId();
	if (devId < 0)
	{
		close(m_smbusfd);
		throw PoeControllerError(std::strerror(errno));
	}
	else if (devId != 0x44)
	{
		close(m_smbusfd);
		throw PoeControllerError("Invalid device ID found");
	}
}

Pd69104::~Pd69104()
{
	if (m_smbusfd > 0) 
		close(m_smbusfd);
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
	return i2c_smbus_read_byte_data(m_smbusfd, kDevIdReg);
}

void Pd69104::setPortEnabled(uint8_t port, bool enabled)
{
	uint8_t data = 0;
	if (enabled) data = (1 << port);
	else data = (1 << (port + 4));

	if (i2c_smbus_write_byte_data(m_smbusfd, kPwrpbReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

uint8_t Pd69104::getPortMode(uint8_t port)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kOpmdReg);
	if (data < 0) 
		throw PoeControllerError(std::strerror(errno));

	//The mode is stored in two bits so lets shift it over until the two bits for our port are the LSBs.
	return ((data >> (port * 2)) & 0b11);
}

void Pd69104::setPortMode(uint8_t port, uint8_t mode)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kOpmdReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	data &= ~(0b11 << (port * 2));			//Make sure both bits for this port are low.
	data |= (mode << (port * 2));			//Then just OR the desired mode (shifted to the correct position) and we are good.

	if (i2c_smbus_write_byte_data(m_smbusfd, kOpmdReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

bool Pd69104::getPortSensing(uint8_t port)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDisenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Sensing is enabled if the ports bit is set in the 4 MSBs or 4 LSBs so we check both.
	//We always only set the 4 LSBs but lets be safe in case someone else has been messing around in the registers.
	return (data & ((1 << (port + 4)) | (1 << port))) != 0;
}

void Pd69104::setPortSensing(uint8_t port, bool sense)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDisenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Bits 4-7 do the same thing as 0-3 on this chip (PD69104).
	//To avoid confusion, let's only work with bits 0-3 and always keeps bits 4-7 low.
	data &= 0x0F;
	if (sense) data |= (1 << port);
	else data &= ~(1 << port);

	if (i2c_smbus_write_byte_data(m_smbusfd, kDisenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

bool Pd69104::getPortDetection(uint8_t port)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	return (data & (1 << port)) != 0;
}

void Pd69104::setPortDetection(uint8_t port, bool detect)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	if (detect) data |= (1 << port);
	else data &= ~(1 << port);

	if (i2c_smbus_write_byte_data(m_smbusfd, kDetenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}


bool Pd69104::getPortClassification(uint8_t port)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Left shift by 4 since Classification is stored in the 4 MSBs
	return (data & (1 << (port + 4))) != 0;
}

void Pd69104::setPortClassification(uint8_t port, bool classify)
{
	int data = i2c_smbus_read_byte_data(m_smbusfd, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Classification and Detection are in the same register. 
	//4 MSBs are for Classification so we need to shift our bitmask.
	if (classify) data |= (1 << (port + 4));
	else data &= ~(1 << (port + 4));

	if (i2c_smbus_write_byte_data(m_smbusfd, kDetenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}