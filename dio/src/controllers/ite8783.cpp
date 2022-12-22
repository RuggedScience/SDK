#include "ite8783.h"

#include <iostream>

#ifdef __linux__
#include <sys/io.h>
#elif _WIN32
#include "../../../utils/portio.hpp"
#endif

static const uint16_t kSpecialAddress =
    0x002E;  // MMIO of the SuperIO's address port. Set this to the value of the
             // register in the SuperIO that you would like to change / read.
static const uint16_t kSpecialData =
    0x002F;  // MMIO of the SuperIO's data port. Use the register to read / set
             // the data for whatever value SPECIAL_ADDRESS was set to.
static const uint8_t kLdnRegister =
    0x07;  // SuperIo register that holds the current logical device number in
           // the SuperIO.
static const uint8_t kGpioLdn =
    0x07;  // The logical device number for most GPIO registers.

static const uint8_t kChipIdRegisterH =
    0x20;  // SuperIO register that holds the high byte of the chip ID.
static const uint8_t kChipIdRegisterL =
    0x21;  // SuperIO register that holds the low byte of the chip ID.
static const uint8_t kGpioBaseRegisterH =
    0x62;  // SuperIO register that holds the high byte of the base address
           // register (BAR) for the GPIO pins.
static const uint8_t kGpioBaseRegisterL =
    0x63;  // SuperIO register that holds the low byte of the base address
           // register (BAR) for the GPIO pins.

static const uint8_t kPolarityBar = 0xB0;
static const uint8_t kPolarityMax = 0xB4;
static const uint8_t kPullUpBar = 0xB8;
static const uint8_t kPullupMax = 0xBC;
static const uint8_t kSimpleIoBar = 0xC0;
static const uint8_t kSimpleIoMax = 0xC4;
static const uint8_t kOutputEnableBar = 0xC8;
static const uint8_t kOutputEnableMax = 0xCD;

Ite8783::Ite8783(bool debug) : AbstractDioController(), m_baseAddress(0)
{
    enterSio();

    try {
        setSioLdn(kGpioLdn);

        uint16_t chipId = getChipId();

        if (debug)
            std::cout << "Hardware Controller ID: 0x" << std::hex << (int)chipId
                      << std::endl;

        if (chipId != 0x8783)
            throw std::system_error(
                std::make_error_code(std::errc::no_such_device)
            );

        m_baseAddress = getBaseAddressRegister();

        if (debug)
            std::cout << "Found base address register of 0x" << std::hex
                      << (int)m_baseAddress << std::endl;
    }
    catch (...) {
        exitSio();
        throw;
    }
}

Ite8783::~Ite8783() { exitSio(); }

void Ite8783::initPin(const PinConfig &config)
{
    setSioLdn(kGpioLdn);
    uint8_t reg = kPolarityBar + config.offset;
    if (reg <= kPolarityMax)
        writeSioRegister(
            reg, readSioRegister(reg) & ~config.bitmask
        );  // Set polarity to non-inverting

    reg = kSimpleIoBar + config.offset;
    if (reg <= kSimpleIoMax)
        writeSioRegister(
            reg, readSioRegister(reg) | config.bitmask
        );  // Set pin as "Simple I/O" instead of "Alternate function"

    if (config.supportsInput)
        setPinMode(config, ModeInput);
    else
        setPinMode(config, ModeOutput);
}

PinMode Ite8783::getPinMode(const PinConfig &config)
{
    setSioLdn(kGpioLdn);
    uint8_t reg = kOutputEnableBar + config.offset;
    uint8_t data = readSioRegister(reg);
    if ((data & config.bitmask) == config.bitmask)
        return ModeOutput;
    else
        return ModeInput;
}

void Ite8783::setPinMode(const PinConfig &config, PinMode mode)
{
    if (mode == ModeInput && !config.supportsInput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Input mode not supported on pin"
        );

    if (mode == ModeOutput && !config.supportsOutput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Output mode not supported on pin"
        );

    setSioLdn(kGpioLdn);
    uint8_t reg = kOutputEnableBar + config.offset;
    uint8_t data = readSioRegister(reg);
    if (mode == ModeInput)
        data &= ~config.bitmask;
    else if (mode == ModeOutput)
        data |= config.bitmask;
    writeSioRegister(reg, data);
}

bool Ite8783::getPinState(const PinConfig &config)
{
    uint16_t reg = m_baseAddress + config.offset;
    if (ioperm(reg, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    bool state = false;
    uint8_t data = inb(reg);
    ioperm(reg, 1, 0);
    if ((data & config.bitmask) == config.bitmask)
        state = true;
    else
        state = false;

    if (config.invert) state = !state;

    return state;
}

void Ite8783::setPinState(const PinConfig &config, bool state)
{
    if (!config.supportsOutput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Output mode not supported on pin"
        );

    if (getPinMode(config) != ModeOutput)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument),
            "Can't set state of pin in input mode"
        );

    if (config.invert) state = !state;
    uint16_t reg = m_baseAddress + config.offset;
    if (ioperm(reg, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    uint8_t data = inb(reg);
    if (state)
        data |= config.bitmask;
    else
        data &= ~config.bitmask;

    outb(data, reg);
    ioperm(reg, 1, 0);
}

void Ite8783::printRegs() {}

// Special series of data that must be written to a specific memory address to
// enable access to the SuperIo's configuration registers.
void Ite8783::enterSio()
{
    if (ioperm(0x2E, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    outb(0x87, 0x2E);
    outb(0x01, 0x2E);
    outb(0x55, 0x2E);
    outb(0x55, 0x2E);

    ioperm(0x2E, 1, 0);
}

void Ite8783::exitSio()
{
    if (ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    outb(0x02, kSpecialAddress);
    outb(0x02, kSpecialData);

    ioperm(kSpecialAddress, 2, 0);
}

uint8_t Ite8783::readSioRegister(uint8_t reg)
{
    if (ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    outb(reg, kSpecialAddress);
    return inb(kSpecialData);

    ioperm(kSpecialAddress, 2, 0);
}

void Ite8783::writeSioRegister(uint8_t reg, uint8_t data)
{
    if (ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    outb(reg, kSpecialAddress);
    outb(data, kSpecialData);

    ioperm(kSpecialAddress, 2, 0);
}

// The SuperIo uses logical device numbers (LDN) to "multiplex" registers.
// The correct LDN must be set to access certain registers.
// Really just here for readability.
void Ite8783::setSioLdn(uint8_t ldn) { writeSioRegister(kLdnRegister, ldn); }

uint16_t Ite8783::getChipId()
{
    uint16_t id = readSioRegister(kChipIdRegisterH) << 8;
    id |= readSioRegister(kChipIdRegisterL);
    return id;
}

uint16_t Ite8783::getBaseAddressRegister()
{
    uint16_t bar = readSioRegister(kGpioBaseRegisterH) << 8;
    bar |= readSioRegister(kGpioBaseRegisterL);
    return bar;
}