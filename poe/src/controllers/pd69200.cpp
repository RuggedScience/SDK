#include "pd69200.h"

#include <fcntl.h>

#include <cstring>
#include <system_error>
#include <thread>

#include "../../../utils/i801_smbus.h"

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif  // DEBUG

#define COMMAND_KEY 0x00
#define PROGRAM_KEY 0x01
#define REQUEST_KEY 0x02
#define TELEMETRY_KEY 0x03
#define REPORT_KEY 0x52

#define MSG_KEY(msg) msg[0]
#define MSG_ECHO(msg) msg[1]
#define MSG_CHKSUM_L(msg) msg[MSG_LEN - 1]
#define MSG_CHKSUM_H(msg) msg[MSG_LEN - 2]

#define PD69200_ID 0x16
#define PD69220_ID 0x1C

typedef std::chrono::nanoseconds ns_t;
typedef std::chrono::milliseconds ms_t;

// Get Software Version
static const msg_t softwareVersionCmd = {
    0x02,
    0x00,
    0x07,
    0x1E,
    0x21,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

// Set Enable/Disable Channels
static const msg_t setEnabledCmd = {
    0x00,
    0x00,
    0x05,
    0x0C,
    0x00,
    0x00,
    0x02,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t getStatusCmd = {
    0x02,
    0x00,
    0x05,
    0x0E,
    0x00,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t setForceCmd = {
    0x00,
    0x00,
    0x05,
    0x51,
    0x00,
    0x00,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t getMeasurementsCmd = {
    0x02,
    0x00,
    0x05,
    0x01,
    0x00,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t getTotalPowerCmd = {
    0x02,
    0x00,
    0x07,
    0x0B,
    0x60,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t setPowerBanksCmd = {
    0x00,
    0x00,
    0x07,
    0x0B,
    0x57,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
};

static const msg_t getPowerBanksCmd = {
    0x02,
    0x00,
    0x07,
    0x0B,
    0x57,
    0x00,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static const msg_t commandAccepted = {
    0x52,
    0x00,
    0x00,
    0x00,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E,
    0x4E
};

static uint16_t calcCheckSum(const uint8_t *msg, size_t size)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < size; ++i) sum += msg[i];

    return sum;
}

Pd69200::Pd69200(uint16_t bus, uint8_t dev, uint16_t totalBudget)
    : AbstractPoeController(),
      m_busAddr(bus),
      m_devAddr(dev),
      m_lastEcho(0),
      m_lastCommandTime()
{
    // There is an edge case that happens if there was any sort of error on a
    // previous transaction. The controller will store the responses and send
    // those before responding to our message. We need to clear all of the old
    // responses by reading them all. Once we recieve an entire empty response,
    // we should be good.
    int count = 0;
    while (count++ < MSG_LEN) {
        if (smbus_read(bus, dev) != 0) count = 0;
    }

    m_devId = getDeviceId();

    if (m_devId != PD69200_ID && m_devId != PD69220_ID)
        throw std::system_error(std::make_error_code(std::errc::no_such_device)
        );

    PowerBankSettings s = getPowerBankSettings(0);
    if (s.powerLimit != totalBudget) {
        s.powerLimit = totalBudget;
        setPowerBankSettings(0, s);
    }
}

Pd69200::~Pd69200() {}

rs::PoeState Pd69200::getPortState(uint8_t port)
{
    if (port == 0x80)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument), "Invalid port"
        );

    PortStatus status = getPortStatus(port);

    if (!status.enabled)
        return rs::PoeState::Disabled;
    else if (status.force)
        return rs::PoeState::Enabled;
    else
        return rs::PoeState::Auto;
}

void Pd69200::setPortState(uint8_t port, rs::PoeState state)
{
    switch (state) {
        case rs::PoeState::Enabled:
            setPortEnabled(port, true);
            setPortForce(port, true);
            break;
        case rs::PoeState::Disabled:
            setPortForce(port, false);
            setPortEnabled(port, false);
            break;
        case rs::PoeState::Auto:
            setPortEnabled(port, true);
            setPortForce(port, false);
            break;
        case rs::PoeState::Error:
            throw std::system_error(
                std::make_error_code(std::errc::invalid_argument),
                "Invalid PoE state"
            );
    }
}

float Pd69200::getPortVoltage(uint8_t port)
{
    if (port == 0x80)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument), "Invalid port"
        );

    return getPortMeasurements(port).voltage;
}

float Pd69200::getPortCurrent(uint8_t port)
{
    if (port == 0x80)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument), "Invalid port"
        );

    return getPortMeasurements(port).current;
}

float Pd69200::getPortPower(uint8_t port)
{
    if (port == 0x80)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument), "Invalid port"
        );

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

int Pd69200::getBudgetTotal() { return getSystemMeasuerments().budgetedWatts; }

msg_t Pd69200::sendMsgToController(msg_t &msg)
{
    msg[1] = m_lastEcho++;
    if (m_lastEcho > 0xFE)
        m_lastEcho = 0;  // According to docs echo shouldn't exceed 0xFE.

    uint16_t chksum = calcCheckSum(msg.data(), MSG_LEN - 2);
    uint8_t chksum_h = chksum >> 8;
    uint8_t chksum_l = chksum & 0xFF;

    MSG_CHKSUM_H(msg) = chksum_h;
    MSG_CHKSUM_L(msg) = chksum_l;

#ifdef DEBUG
    std::cout << "Sending message to controller" << std::endl;
    for (size_t i = 0; i < MSG_LEN; ++i) {
        std::cout << std::hex << "0x" << (int)msg[i];
        if (i < MSG_LEN - 1)
            std::cout << ", ";
        else
            std::cout << std::endl;
    }
#endif  // DEBUG

    // See table 1-2 from the PD692x0 serial communication protocol user guide.
    // According to the serial communication guide, we need to wait 30ms between
    // commands. A command is a message with a key of 0x00 and the key is the
    // first byte in the message. So check if we are sending a command.
    if (MSG_KEY(msg) == COMMAND_KEY) {
        // Calculate how long it's been since the last time we sent a command.
        ns_t deltaTime = clock_timer_t::now() - m_lastCommandTime;
        ns_t timeLeft = ms_t(30) - deltaTime;
        // If we have time left, we need to sleep for that much time.
        if (timeLeft.count() > 0) {
            std::this_thread::sleep_for(timeLeft);
        }
    }

    for (size_t i = 0; i < MSG_LEN; ++i) {
        smbus_write(m_busAddr, m_devAddr, msg[i]);
    }

    // See table 1-2 from the PD692x0 serial communication protocol user guide.
    // We have to wait 30ms after sending a message before we can read back the
    // response.
    std::this_thread::sleep_for(ms_t(30));

    msg_t response;
    for (size_t i = 0; i < MSG_LEN; ++i) {
        response[i] = smbus_read(m_busAddr, m_devAddr);
    }

    // As described above, we need to wait between command messages.
    // Log the time we sent the last command so we can make sure we do this.
    if (MSG_KEY(msg) == COMMAND_KEY) {
        m_lastCommandTime = clock_timer_t::now();
    }

#ifdef DEBUG
    std::cout << "Received message from controller" << std::endl;
    for (size_t i = 0; i < MSG_LEN; ++i) {
        std::cout << std::hex << "0x" << (int)response[i];
        if (i < MSG_LEN - 1)
            std::cout << ", ";
        else
            std::cout << std::endl;
    }
#endif  // DEBUG

    chksum = (MSG_CHKSUM_H(response) << 8) | MSG_CHKSUM_L(response);
    if (chksum != calcCheckSum(response.data(), MSG_LEN - 2))
        throw std::system_error(
            std::make_error_code(std::errc::protocol_error), "Invalid checksum"
        );

    if (MSG_ECHO(msg) != MSG_ECHO(response))
        throw std::system_error(
            std::make_error_code(std::errc::protocol_error), "Invalid echo"
        );

    // If the msg is a command or program we should expect a success response
    // from the controller.
    if (MSG_KEY(msg) == COMMAND_KEY || MSG_KEY(msg) == PROGRAM_KEY) {
        for (size_t i = 0; i < MSG_LEN - 2; ++i) {
            if (i == 1) continue;  // Ignore echo byte.

            if (response[i] != commandAccepted[i])
                throw std::system_error(
                    std::make_error_code(std::errc::protocol_error),
                    "Command unsuccessful"
                );
        }
    }
    // If the msg is a request we should expect a telemetry response from the
    // controller.
    else if (MSG_KEY(msg) == REQUEST_KEY) {
        if (MSG_KEY(response) != TELEMETRY_KEY)
            throw std::system_error(
                std::make_error_code(std::errc::protocol_error),
                "Invalid telemetry key"
            );
    }

    return response;
}

uint8_t Pd69200::getDeviceId()
{
    msg_t response, msg = softwareVersionCmd;
    response = sendMsgToController(msg);

    return response[4];
}

Pd69200::PortStatus Pd69200::getPortStatus(uint8_t port)
{
    msg_t response, msg = getStatusCmd;
    msg[4] = port;
    response = sendMsgToController(msg);

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
    msg_t msg = setEnabledCmd;
    msg[4] = port;
    msg[5] = enable ? 0x01 : 0x00;
    msg[6] = enable ? 0x01 : 0x02;
    sendMsgToController(msg);
}

void Pd69200::setPortForce(uint8_t port, bool force)
{
    msg_t msg = setForceCmd;
    msg[4] = port;
    msg[5] = force ? 0x01 : 0x00;
    sendMsgToController(msg);
}

Pd69200::PortMeasurements Pd69200::getPortMeasurements(uint8_t port)
{
    msg_t response, msg = getMeasurementsCmd;
    msg[4] = port;
    response = sendMsgToController(msg);

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

Pd69200::PowerBankSettings Pd69200::getPowerBankSettings(uint8_t bank)
{
    msg_t response, msg = getPowerBanksCmd;
    msg[5] = bank;
    response = sendMsgToController(msg);

    uint16_t val;
    PowerBankSettings s;

    val = (response[2] << 8) | response[3];
    s.powerLimit = (int)val;

    val = (response[4] << 8) | response[5];
    s.maxShutdownVoltage = val / 10.0f;

    val = (response[6] << 8) | response[7];
    s.minShutdownVoltage = val / 10.0f;

    s.guardBand = response[8];
    if (msg[9] >= 0x03)
        s.sourceType = SourceTypeReserved;
    else
        s.sourceType = (PowerBankSourceType)(msg[9] & 0x03);

    return s;
}

void Pd69200::setPowerBankSettings(
    uint8_t bank,
    const Pd69200::PowerBankSettings &settings
)
{
    msg_t msg = setPowerBanksCmd;
    msg[5] = bank;

    uint16_t val;

    msg[6] = (uint8_t)(settings.powerLimit >> 8);
    msg[7] = (uint8_t)settings.powerLimit;

    val = (uint16_t)settings.maxShutdownVoltage * 10;
    msg[8] = (uint8_t)(val >> 8);
    msg[9] = (uint8_t)val;

    val = (uint16_t)settings.minShutdownVoltage * 10;
    msg[10] = (uint8_t)(val >> 8);
    msg[11] = (uint8_t)val;

    msg[12] = settings.guardBand;

    sendMsgToController(msg);
}