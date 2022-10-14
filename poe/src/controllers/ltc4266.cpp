#include "ltc4266.h"
#include "../../utils/i801_smbus.h"

#include <stdexcept>
#include <fcntl.h>
#include <cstring>

static const uint8_t kDeviceId = 0x64;

//Registers as described in the datasheet for the PD69104.
static const uint8_t kSataPwrReg = 0x10;	//Power Status register
static const uint8_t kOpmdReg = 0x12;		//Operating Mode register
static const uint8_t kDisenaReg = 0x13;     //Disconnect Sensing Enable register
static const uint8_t kDetenaReg = 0x14;     //Detection and Classification Enable register
static const uint8_t kPwrpbReg = 0x19;		//Power On/Off Pushbutton register
static const uint8_t kDevIdReg = 0x1B;		//Device ID register.

static const float kVoltsCoef = 5.835f;
static const uint8_t kPort1VoltReg = 0x32;
static const uint8_t kPort2VoltReg = 0x36;
static const uint8_t kPort3VoltReg = 0x3A;
static const uint8_t kPort4VoltReg = 0x3E;

static const float kCurCoef = 122.07f;
static const uint8_t kPort1CurReg = 0x30;
static const uint8_t kPort2CurReg = 0x34;
static const uint8_t kPort3CurReg = 0x38;
static const uint8_t kPort4CurReg = 0x3C;

static const uint8_t kShutdownMode = 0;
static const uint8_t kManualMode =	1;
static const uint8_t kSemiAutoMode = 2;
static const uint8_t kAutoMode = 3;

Ltc4266::Ltc4266(uint16_t bus, uint8_t dev) :
	AbstractPoeController(),
	m_busAddr(bus),
	m_devAddr(dev)
{
	int devId = getDeviceId();
	if (devId < 0)
		throw PoeControllerError(std::strerror(errno));
	else if (devId != kDeviceId)
		throw PoeControllerError("Invalid device ID found");
}

Ltc4266::~Ltc4266()
{

}

PoeState Ltc4266::getPortState(uint8_t port)
{
	uint8_t mode = getPortMode(port);
	if (mode == kManualMode)
		return StateEnabled;
	else if (mode == kShutdownMode)
		return StateDisabled;
	else if (mode == kAutoMode)
		return StateAuto;
	else
		throw PoeControllerError("Unknown port mode found");
}

void Ltc4266::setPortState(uint8_t port, PoeState state)
{
	switch (state)
	{
		case StateEnabled:
			setPortMode(port, kManualMode);
			setPortDetection(port, false);
			setPortClassification(port, false);
			setPortSensing(port, false);
			setPortEnabled(port, true);
			break;
		case StateDisabled:
			setPortMode(port, kShutdownMode);
			break;
		case StateAuto:
			setPortMode(port, kAutoMode);
			setPortDetection(port, true);
			setPortClassification(port, true);
			setPortSensing(port, true);
			break;
		case StateError:
			throw PoeControllerError("Invalid PoeState: StateError");
	}
}

float Ltc4266::getPortVoltage(uint8_t port)
{
	uint8_t reg = 0;
	if (port == 0) reg = kPort1VoltReg;
	else if (port == 1) reg = kPort2VoltReg;
	else if (port == 2) reg = kPort3VoltReg;
	else if (port == 3) reg = kPort4VoltReg;

	if (reg == 0)
		throw PoeControllerError("Invalid Port");

	int data = smbusReadRegister(m_busAddr, m_devAddr, reg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	uint16_t volts = 0x00FF & data;

	data = smbusReadRegister(m_busAddr, m_devAddr, reg+1);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	volts |= data << 8;

	return (volts * kVoltsCoef) / 1000.0f; // Convert from mV to V
}

float Ltc4266::getPortCurrent(uint8_t port)
{
	uint8_t reg = 0;
	if (port == 0) reg = kPort1CurReg;
	else if (port == 1) reg = kPort2CurReg;
	else if (port == 2) reg = kPort3CurReg;
	else if (port == 3) reg = kPort4CurReg;

	if (reg == 0)
		throw PoeControllerError("Invalid Port");

	int data = smbusReadRegister(m_busAddr, m_devAddr, reg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	uint16_t cur = 0x00FF & data;

	data = smbusReadRegister(m_busAddr, m_devAddr, reg+1);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	cur |= data << 8;

	return (cur * kCurCoef) / 1000000.0f; // Convert from uA to A
}

int Ltc4266::getBudgetConsumed()
{
	float power, consumed = 0.0f;
	for (uint8_t i = 0; i < 4; ++i)
	{
		power = getPortPower(i);
		if (power < 0)
			throw PoeControllerError(std::strerror(errno));
		
		consumed += power;
	}

	return (int)consumed;
}

int Ltc4266::getDeviceId() const
{
	return smbusReadRegister(m_busAddr, m_devAddr, kDevIdReg);
}

void Ltc4266::setPortEnabled(uint8_t port, bool enabled)
{
	uint8_t data = 0;
	if (enabled) data = (1 << port);
	else data = (1 << (port + 4));

	if (smbusWriteRegister(m_busAddr, m_devAddr, kPwrpbReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

uint8_t Ltc4266::getPortMode(uint8_t port) const
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kOpmdReg);
	if (data < 0) 
		throw PoeControllerError(std::strerror(errno));

	//The mode is stored in two bits so lets shift it over until the two bits for our port are the LSBs.
	return ((data >> (port * 2)) & 0b11);
}

void Ltc4266::setPortMode(uint8_t port, uint8_t mode)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kOpmdReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	data &= ~(0b11 << (port * 2));			//Make sure both bits for this port are low.
	data |= (mode << (port * 2));			//Then just OR the desired mode (shifted to the correct position) and we are good.

	if (smbusWriteRegister(m_busAddr, m_devAddr, kOpmdReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}

bool Ltc4266::getPortSensing(uint8_t port) const
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDisenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Sensing is enabled if the ports bit is set in the 4 MSBs or 4 LSBs so we check both.
	//We always only set the 4 LSBs but lets be safe in case someone else has been messing around in the registers.
	return (data & ((1 << (port + 4)) | (1 << port))) != 0;
}

void Ltc4266::setPortSensing(uint8_t port, bool sense)
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

bool Ltc4266::getPortDetection(uint8_t port) const
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	return (data & (1 << port)) != 0;
}

void Ltc4266::setPortDetection(uint8_t port, bool detect)
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));

	if (detect) data |= (1 << port);
	else data &= ~(1 << port);

	if (smbusWriteRegister(m_busAddr, m_devAddr, kDetenaReg, data) < 0)
		throw PoeControllerError(std::strerror(errno));
}


bool Ltc4266::getPortClassification(uint8_t port) const
{
	int data = smbusReadRegister(m_busAddr, m_devAddr, kDetenaReg);
	if (data < 0)
		throw PoeControllerError(std::strerror(errno));
	//Left shift by 4 since Classification is stored in the 4 MSBs
	return (data & (1 << (port + 4))) != 0;
}

void Ltc4266::setPortClassification(uint8_t port, bool classify)
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