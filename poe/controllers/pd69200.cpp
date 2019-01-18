#include "pd69200.h"

#include "../../utils/i801_smbus.hpp"

#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <vector>

#define MSG_LEN     15
#define CHKSUM_L    MSG_LEN - 1
#define CHKSUM_H    MSG_LEN - 2

#define MSG_HIGH 14
#define SUM_IDX  13

static const uint8_t kDeviceId = 0x16;

// Get Software Version
static const std::vector<uint8_t> softwareVersionCmd({
    0x02, 0x00, 0x07, 0x1E, 0x21, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
});

// Get All Ports Enable/Disable
static const std::vector<uint8_t> getPortsCmd({
    0x02, 0x00, 0x07, 0x0C, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
});

// Set Enable/Disable Channels
static const std::vector<uint8_t> setPortsCmd({
    0x00, 0x00, 0x05, 0x0C, 0x00, 0x00, 0x02, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
});

static const std::vector<uint8_t> getPortSts({
    0x02, 0x00, 0x05, 0x0E, 0x00, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E, 0x4E
});

static uint16_t calcCheckSum(const uint8_t *msg, size_t size)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < size; ++i)
        sum += msg[i];

    return sum;
}

static uint16_t calcCheckSum(const std::vector<uint8_t>& msg)
{
    uint16_t sum = 0;
    std::vector<uint8_t>::const_iterator i;
    for (i = msg.begin(); i != msg.end(); ++i)
        sum += (*i);

    return sum;
}

Pd69200::Pd69200(uint16_t bus, uint8_t dev) :
	AbstractPoeController(),
	m_busAddr(bus),
	m_devAddr(dev),
    m_lastEcho(0)
{
	int devId = getDeviceId();
	if (devId < 0 || devId != kDeviceId)
		throw PoeControllerError(std::strerror(errno));
}

Pd69200::~Pd69200()
{
    
}

PoeState Pd69200::getPortState(uint8_t port)
{
    bool enabled = getPortEnabled(port);
    bool force = getPortForce(port);

    if (!enabled) return StateDisabled;
    else if (force) return StateEnabled;
    else return StateAuto;
}

void Pd69200::setPortState(uint8_t port, PoeState state)
{
    throw PoeControllerError("Operation not supported");
}

float Pd69200::getPortVoltage(uint8_t port)
{
    throw PoeControllerError("Operation not supported");
}

float Pd69200::getPortCurrent(uint8_t port)
{
    throw PoeControllerError("Operation not supported");
}

void Pd69200::sendMsgToController(std::vector<uint8_t>& msg, bool response)
{
    msg[1] = m_lastEcho++;
    uint16_t chksum = calcCheckSum(msg);
    uint8_t chksum_h = chksum >> 8;
    uint8_t chksum_l = chksum & 0xFF;

    msg.emplace_back(chksum_h);

    if (!response)
        msg.emplace_back(chksum_l);

    std::vector<uint8_t>::iterator i;
    for (i = msg.begin(); i != msg.end(); ++i)
    {
        if (smbusWriteByte(m_busAddr, m_devAddr, (*i)) < 0)
            throw PoeControllerError(std::strerror(errno));
    }

    if (response)
    {
        uint8_t data[MSG_LEN] = { 0 };
        if (smbusI2CRead(m_busAddr, m_devAddr, chksum_l, data, MSG_LEN) < 0)
            throw PoeControllerError(std::strerror(errno));

        chksum = (data[MSG_LEN - 2] << 8) | data[MSG_LEN - 1];
        if (chksum != calcCheckSum(data, MSG_LEN - 2))
            throw PoeControllerError("Invalid data received from controller");

        msg.assign(&data, &data + MSG_LEN);
    }
}

int Pd69200::getDeviceId()
{
    std::vector<uint8_t> msg(softwareVersionCmd);
    sendMsgToController(msg, true);
    return msg.at(4);
}

bool Pd69200::getPortEnabled(uint8_t port)
{
    std::vector<uint8_t> msg(getPortsCmd);
    sendMsgToController(msg, true);

    if (port < 8)
        return (msg.at(2) >> port) == 1;
    else if (port < 16)
        return (msg.at(3) >> (port - 8) == 1);
    else
        throw PoeControllerError("Port not supported");
}

bool Pd69200::getPortForce(uint8_t port)
{
    std::vector<uint8_t> msg(getPortSts);
    msg[4] = port;
    sendMsgToController(msg, true);
    return msg[4] == 1;
}