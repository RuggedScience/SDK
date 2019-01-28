#include "pd69200.h"

#include "../../utils/i801_smbus.h"

#include <stdexcept>
#include <fcntl.h>
#include <cstring>

#define CHKSUM_L    MSG_LEN - 1
#define CHKSUM_H    MSG_LEN - 2

static const uint8_t kDeviceId = 0x16;

// Get Software Version
static const msg_t softwareVersionCmd = {
    0x02, 0x00, 0x07, 0x1E, 0x21, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

// Set Enable/Disable Channels
static const msg_t setEnabledCmd = {
    0x00, 0x00, 0x05, 0x0C, 0x00, 0x00, 0x02, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static const msg_t getStatusCmd = {
    0x02, 0x00, 0x05, 0x0E, 0x00, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static const msg_t setForceCmd = {
    0x00, 0x00, 0x05, 0x51, 0x00, 0x00, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static const msg_t getMeasurementsCmd = {
    0x02, 0x00, 0x05, 0x01, 0x00, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static const msg_t getTotalPowerCmd = {
    0x02, 0x00, 0x07, 0x0B, 0x60, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static const msg_t commandAccepted = {
    0x52, 0x00, 0x00, 0x00, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
};

static uint16_t calcCheckSum(const uint8_t *msg, size_t size)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < size; ++i)
        sum += msg[i];

    return sum;
}

Pd69200::Pd69200(uint16_t bus, uint8_t dev) :
	AbstractPoeController(),
	m_busAddr(bus),
	m_devAddr(dev),
    m_lastEcho(0)
{
    int devId;
    try { devId = getDeviceId(); }
    catch (PoeControllerError) { devId = getDeviceId(); }
    
	if (devId < 0 || devId != kDeviceId)
		throw PoeControllerError(std::strerror(errno));
}

Pd69200::~Pd69200()
{
    
}

PoeState Pd69200::getPortState(uint8_t port)
{
    if (port == 0x80)
        throw PoeControllerError("Can't get state for all ports");

    PortStatus status = getPortStatus(port);

    if (!status.enabled) return StateDisabled;
    else if (status.force) return StateEnabled;
    else return StateAuto;
}

void Pd69200::setPortState(uint8_t port, PoeState state)
{
    switch (state)
    {
        case StateEnabled:
            setPortEnabled(port, true);
            setPortForce(port, true);
            break;
        case StateDisabled:
            setPortForce(port, false);
            setPortEnabled(port, false);
            break;
        case StateAuto:
            setPortEnabled(port, true);
            setPortForce(port, false);
            break;
        case StateError:
            break;
    }
}

float Pd69200::getPortVoltage(uint8_t port)
{
    if (port == 0x80)
        throw PoeControllerError("Can't get voltage for all ports");

    return getPortMeasurements(port).voltage;
}

float Pd69200::getPortCurrent(uint8_t port)
{
    if (port == 0x80)
        throw PoeControllerError("Can't get current of all ports");

    return getPortMeasurements(port).current;
}

float Pd69200::getPortPower(uint8_t port)
{
    if (port == 0x80)
        throw PoeControllerError("Use getBudgetConsumed for total power");

    return getPortMeasurements(port).wattage;
}

int Pd69200::getBudgetConsumed()
{
    return getSystemMeasuerments().calculatedWatts;
}

int Pd69200::getBudgetAvailable()
{
    return getSystemMeasuerments().availableWatts;
}

int Pd69200::getBudgetTotal()
{
    return getSystemMeasuerments().budgetedWatts;
}

msg_t Pd69200::sendMsgToController(msg_t& msg)
{
    msg[1] = m_lastEcho++;
    if (m_lastEcho > 0xFE) m_lastEcho = 0; // According to docs echo shouldn't exceed 0xFE.

    uint16_t chksum = calcCheckSum(msg.data(), MSG_LEN - 2);
    uint8_t chksum_h = chksum >> 8;
    uint8_t chksum_l = chksum & 0xFF;

    msg[CHKSUM_H] = chksum_h;
    msg[CHKSUM_L] = chksum_l;

    for (size_t i = 0; i < MSG_LEN - 1; ++i)    // Send everything but the last byte.
    {
        if (smbusWriteByte(m_busAddr, m_devAddr, msg[i]) < 0)
            throw PoeControllerError(std::strerror(errno));
    }

    uint16_t len = 0;
    msg_t response;
    if (smbusI2CRead(m_busAddr, m_devAddr, msg[MSG_LEN - 1], response.data(), response.size()) < 0) // Now we can send the last byte
        throw PoeControllerError(std::strerror(errno));

    chksum = (response[MSG_LEN - 2] << 8) | response[MSG_LEN - 1];
    if (chksum != calcCheckSum(response.data(), MSG_LEN - 2))
        throw PoeControllerError("Invalid checksum received from controller");

    if (msg[1] != response[1])
        throw PoeControllerError("Invalid echo received from controller");
    
    return response;
}

int Pd69200::getDeviceId()
{
    msg_t response, msg = softwareVersionCmd;
    response = sendMsgToController(msg);

    if (response[0] != 0x03)
        throw PoeControllerError("Invalid response received from controller");
        
    return response[4];
}

Pd69200::PortStatus Pd69200::getPortStatus(uint8_t port)
{
    msg_t response, msg = getStatusCmd;
    msg[4] = port;
    response = sendMsgToController(msg);

    if (response[0] != 0x03)
        throw PoeControllerError("Invalid response received from controller");
    
    PortStatus s;
    s.enabled = ((response[2] & 0x01) == 0x01);
    s.status = response[3];
    s.force = (response[4] == 0x01);
    s.latch = response[5];
    s.classType = response[6];
    s.mode = response[10];
    s.fourPair = (response[11] == 0x01);

    return s;
}

void Pd69200::setPortEnabled(uint8_t port, bool enable)
{
    msg_t response, msg = setEnabledCmd;
    msg[4] = port;
    msg[5] = enable ? 0x01 : 0x00;
    msg[6] = enable ? 0x01 : 0x02;
    response = sendMsgToController(msg);

    for (size_t i = 0; i < MSG_LEN - 2; ++i)
    {
        if (i == 1) continue; // Ignore echo byte.

        if (response[i] != commandAccepted[i])
            throw PoeControllerError("Invalid response received from controller");
    }
}

void Pd69200::setPortForce(uint8_t port, bool force)
{
    msg_t response, msg = setForceCmd;
    msg[4] = port;
    msg[5] = force ? 0x01 : 0x00;
    response = sendMsgToController(msg);

    for (size_t i = 0; i < MSG_LEN - 2; ++i)
    {
        if (i == 1) continue; // Ignore echo byte.
        
        if (response[i] != commandAccepted[i])
            throw PoeControllerError("Invalid response received from controller");
    }
}

Pd69200::PortMeasurements Pd69200::getPortMeasurements(uint8_t port)
{
    msg_t response, msg = getMeasurementsCmd;
    msg[4] = port;
    response = sendMsgToController(msg);

    if (response[0] != 0x03)
        throw PoeControllerError("Invalid response received from controller");

    uint16_t val;
    PortMeasurements m;

    val = (response[9] << 8) | response[10];
    m.voltage = val * 0.1f;

    val = (response[4] << 8) | response[5];
    m.current = val / 1000.0f;

    val = (response[6] << 8 | response[7]);
    m.wattage = val * 0.005f;

    return m;
}

Pd69200::SystemMeasurements Pd69200::getSystemMeasuerments()
{
    msg_t response, msg = getTotalPowerCmd;
    response = sendMsgToController(msg);

    if (response[0] != 0x03)
        throw PoeControllerError("Invalid response received from controller");

    uint16_t val;
    SystemMeasurements m;

    val = (response[2] << 8) | response[3];
    m.measuredWatts = (int)val;

    val = (response[4] << 8) | response[5];
    m.calculatedWatts = (int)val;

    val = (response[6] << 8 | response[7]);
    m.availableWatts = (int)val;

    val = (response[8] << 8 | response[9]);
    m.budgetedWatts = (int)val;

    return m;
}