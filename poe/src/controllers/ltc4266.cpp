#include "ltc4266.h"
#include "../../../utils/i801_smbus.h"

#include <fcntl.h>
#include <cstring>
#include <system_error>

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
	if (devId != kDeviceId)
		throw std::system_error(std::make_error_code(std::errc::no_such_device));
}

Ltc4266::~Ltc4266()
{

}

rs::PoeState Ltc4266::getPortState(uint8_t port)
{
	uint8_t mode = getPortMode(port);
	if (mode == kManualMode)
		return rs::PoeState::Enabled;
	else if (mode == kShutdownMode)
		return rs::PoeState::Disabled;
	else if (mode == kAutoMode)
		return rs::PoeState::Auto;
	else
		throw std::system_error(std::make_error_code(std::errc::protocol_error), "Received invalid data from controller");
}

void Ltc4266::setPortState(uint8_t port, rs::PoeState state)
{
	switch (state)
	{
		case rs::PoeState::Enabled:
			setPortMode(port, kManualMode);
			setPortDetection(port, false);
			setPortClassification(port, false);
			setPortSensing(port, false);
			setPortEnabled(port, true);
			break;
		case rs::PoeState::Disabled:
			setPortMode(port, kShutdownMode);
			break;
		case rs::PoeState::Auto:
			setPortMode(port, kAutoMode);
			setPortDetection(port, true);
			setPortClassification(port, true);
			setPortSensing(port, true);
			break;
		case rs::PoeState::Error:
			throw std::system_error(std::make_error_code(std::errc::invalid_argument), "Invalid PoE state");
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
		throw std::system_error(std::make_error_code(std::errc::invalid_argument), "Invalid port");

	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, reg);
	uint16_t volts = 0x00FF & data;
	data = smbus_read_register(m_busAddr, m_devAddr, reg+1);
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
		throw std::system_error(std::make_error_code(std::errc::invalid_argument), "Invalid port");

	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, reg);
	uint16_t cur = 0x00FF & data;
	data = smbus_read_register(m_busAddr, m_devAddr, reg+1);
	cur |= data << 8;
	return (cur * kCurCoef) / 1000000.0f; // Convert from uA to A
}

int Ltc4266::getBudgetConsumed()
{
	float power, consumed = 0.0f;
	for (uint8_t i = 0; i < 4; ++i)
	{
		power = getPortPower(i);
		consumed += power;
	}

	return (int)consumed;
}

int Ltc4266::getDeviceId() const
{
	return smbus_read_register(m_busAddr, m_devAddr, kDevIdReg);
}

void Ltc4266::setPortEnabled(uint8_t port, bool enabled)
{
	uint8_t data = 0;
	if (enabled) data = (1 << port);
	else data = (1 << (port + 4));

	smbus_write_register(m_busAddr, m_devAddr, kPwrpbReg, data);
}

uint8_t Ltc4266::getPortMode(uint8_t port) const
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kOpmdReg);

	//The mode is stored in two bits so lets shift it over until the two bits for our port are the LSBs.
	return ((data >> (port * 2)) & 0b11);
}

void Ltc4266::setPortMode(uint8_t port, uint8_t mode)
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kOpmdReg);

	data &= ~(0b11 << (port * 2));			//Make sure both bits for this port are low.
	data |= (mode << (port * 2));			//Then just OR the desired mode (shifted to the correct position) and we are good.

	smbus_write_register(m_busAddr, m_devAddr, kOpmdReg, data);
}

bool Ltc4266::getPortSensing(uint8_t port) const
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDisenaReg);
	//Sensing is enabled if the ports bit is set in the 4 MSBs or 4 LSBs so we check both.
	//We always only set the 4 LSBs but lets be safe in case someone else has been messing around in the registers.
	return (data & ((1 << (port + 4)) | (1 << port))) != 0;
}

void Ltc4266::setPortSensing(uint8_t port, bool sense)
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDisenaReg);
	//Bits 4-7 do the same thing as 0-3 on this chip (PD69104).
	//To avoid confusion, let's only work with bits 0-3 and always keeps bits 4-7 low.
	data &= 0x0F;
	if (sense) data |= (1 << port);
	else data &= ~(1 << port);

	smbus_write_register(m_busAddr, m_devAddr, kDisenaReg, data);
}

bool Ltc4266::getPortDetection(uint8_t port) const
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDetenaReg);
	return (data & (1 << port)) != 0;
}

void Ltc4266::setPortDetection(uint8_t port, bool detect)
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDetenaReg);
	if (detect) data |= (1 << port);
	else data &= ~(1 << port);

	smbus_write_register(m_busAddr, m_devAddr, kDetenaReg, data);
}


bool Ltc4266::getPortClassification(uint8_t port) const
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDetenaReg);
	//Left shift by 4 since Classification is stored in the 4 MSBs
	return (data & (1 << (port + 4))) != 0;
}

void Ltc4266::setPortClassification(uint8_t port, bool classify)
{
	uint8_t data = smbus_read_register(m_busAddr, m_devAddr, kDetenaReg);
	//Classification and Detection are in the same register. 
	//4 MSBs are for Classification so we need to shift our bitmask.
	if (classify) data |= (1 << (port + 4));
	else data &= ~(1 << (port + 4));

	smbus_write_register(m_busAddr, m_devAddr, kDetenaReg, data);
}