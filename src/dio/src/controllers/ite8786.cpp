#include "ite8786.h"

#include <portio.h>

#include <iostream>

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
static const uint8_t kParallelPortLdn = 0x03;

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
static const uint8_t kPullupMax = 0xBD;
static const uint8_t kSimpleIoBar = 0xC0;
static const uint8_t kSimpleIoMax = 0xC4;
static const uint8_t kOutputEnableBar = 0xC8;
static const uint8_t kOutputEnableMax = 0xCF;

Ite8786::Ite8786() : AbstractDioController(), m_currentLdn(0), m_baseAddress(0)
{
    enterSio();

    try {
        setSioLdn(kGpioLdn);

        uint16_t chipId = getChipId();
        // On units with an FSM-100 installed, two SuperIo chips will be
        // present. The FSM-100 uses an IT8783. We don't want to talk to that
        // chip...
        if (chipId == 0x8783) {
            writeSioRegister(
                0x22, 0x80
            );  // Writing 0x80 to register 0x22 will cause the SuperIo on the
                // FSM to exit config mode and should allow us to talk to the
                // SuperIo on the MB.
            chipId = getChipId();
        }

        if (chipId != 0x8786)
            throw std::system_error(
                std::make_error_code(std::errc::no_such_device)
            );

        setSioLdn(kGpioLdn);
        m_baseAddress = getBaseAddressRegister();
    }
    catch (...) {
        exitSio();
        throw;
    }
}

Ite8786::Ite8786(const std::vector<RegisterConfig>& registers)
    : AbstractDioController(), m_currentLdn(0), m_baseAddress(0)
{
    enterSio();

    try {
        setSioLdn(kGpioLdn);

        uint16_t chipId = getChipId();
        // On units with an FSM-100 installed, two SuperIo chips will be
        // present. The FSM-100 uses an IT8783. We don't want to talk to that
        // chip...
        if (chipId == 0x8783) {
            writeSioRegister(
                0x22, 0x80
            );  // Writing 0x80 to register 0x22 will cause the SuperIo on the
                // FSM to exit config mode and should allow us to talk to the
                // SuperIo on the MB.
            chipId = getChipId();
        }

        if (chipId != 0x8786)
            throw std::system_error(
                std::make_error_code(std::errc::no_such_device)
            );

        setSioLdn(kGpioLdn);
        m_baseAddress = getBaseAddressRegister();

        for (const RegisterConfig& reg : registers) {
            setSioLdn(reg.ldn);
            uint8_t oldData = readSioRegister(reg.addr);
            uint8_t newData = oldData | reg.onBits;
            newData &= ~reg.offBits;
            writeSioRegister(reg.addr, newData);
        }
    }
    catch (...) {
        exitSio();
        throw;
    }
}

Ite8786::~Ite8786() { exitSio(); }

void Ite8786::initPin(const DioPinConfig& config)
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

    reg = kPullUpBar + config.offset;
    if (reg <= kPullupMax) {
        uint8_t val = readSioRegister(reg);
        if (config.enablePullup)
            writeSioRegister(reg, val | config.bitmask);
        else
            writeSioRegister(reg, val & ~config.bitmask);
    }

    if (config.supportsInput)
        setPinMode(config, rs::PinDirection::Input);
    else
        setPinMode(config, rs::PinDirection::Output);
}

rs::PinDirection Ite8786::getPinMode(const DioPinConfig& config)
{
    setSioLdn(kGpioLdn);
    uint8_t reg = kOutputEnableBar + config.offset;
    uint8_t data = readSioRegister(reg);
    if ((data & config.bitmask) == config.bitmask)
        return rs::PinDirection::Output;
    else
        return rs::PinDirection::Input;
}

void Ite8786::setPinMode(const DioPinConfig& config, rs::PinDirection mode)
{
    if (mode == rs::PinDirection::Input && !config.supportsInput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Input mode not supported on pin"
        );

    if (mode == rs::PinDirection::Output && !config.supportsOutput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Output mode not supported on pin"
        );

    setSioLdn(kGpioLdn);
    uint8_t reg = kOutputEnableBar + config.offset;
    uint8_t data = readSioRegister(reg);
    if (mode == rs::PinDirection::Input)
        data &= ~config.bitmask;
    else if (mode == rs::PinDirection::Output)
        data |= config.bitmask;
    writeSioRegister(reg, data);
}

bool Ite8786::getPinState(const DioPinConfig& config)
{
    uint16_t reg = m_baseAddress + config.offset;
    if (portio_ioperm(reg, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    bool state = false;
    uint8_t data = portio_inb(reg);
    portio_ioperm(reg, 1, 0);
    if ((data & config.bitmask) == config.bitmask)
        state = true;
    else
        state = false;

    if (config.invert) state = !state;

    return state;
}

void Ite8786::setPinState(const DioPinConfig& config, bool state)
{
    if (!config.supportsOutput)
        throw std::system_error(
            std::make_error_code(std::errc::function_not_supported),
            "Output mode not supported on pin"
        );

    if (getPinMode(config) != rs::PinDirection::Output)
        throw std::system_error(
            std::make_error_code(std::errc::invalid_argument),
            "Can't set state of pin in input mode"
        );

    if (config.invert) state = !state;
    uint16_t reg = m_baseAddress + config.offset;
    if (portio_ioperm(reg, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    uint8_t data = portio_inb(reg);
    if (state)
        data |= config.bitmask;
    else
        data &= ~config.bitmask;

    portio_outb(data, reg);
    portio_ioperm(reg, 1, 0);
}

void Ite8786::printRegs()
{
    setSioLdn(kGpioLdn);

    std::cout << std::endl << "Polarity Registers" << std::endl;
    for (int reg = kPolarityBar; reg <= kPolarityMax; ++reg) {
        int gpio = (reg - kPolarityBar) + 1;
        std::cout << std::hex << "GPIO " << gpio << " (0x" << reg << "):\t0x"
                  << (int)readSioRegister(reg) << std::endl;
    }

    std::cout << std::endl << "Simple I/O Registers" << std::endl;
    for (int reg = kSimpleIoBar; reg <= kSimpleIoMax; ++reg) {
        int gpio = (reg - kSimpleIoBar) + 1;
        std::cout << std::hex << "GPIO " << gpio << " (0x" << reg << "):\t0x"
                  << (int)readSioRegister(reg) << std::endl;
    }

    std::cout << std::endl << "Pullup Enable Registers" << std::endl;
    for (int reg = kPullUpBar; reg <= kPullupMax; ++reg) {
        int gpio = (reg - kPullUpBar) + 1;
        std::cout << std::hex << "GPIO " << gpio << " (0x" << reg << "):\t0x"
                  << (int)readSioRegister(reg) << std::endl;
    }
}

// Special series of data that must be written to a specific memory address to
// enable access to the SuperIo's configuration registers.
void Ite8786::enterSio()
{
    if (portio_ioperm(0x2E, 1, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    portio_outb(0x87, 0x2E);
    portio_outb(0x01, 0x2E);
    portio_outb(0x55, 0x2E);
    portio_outb(0x55, 0x2E);

    portio_ioperm(0x2E, 1, 0);
}

void Ite8786::exitSio()
{
    if (portio_ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    portio_outb(0x02, kSpecialAddress);
    portio_outb(0x02, kSpecialData);

    portio_ioperm(kSpecialAddress, 2, 0);
}

// The SuperIo uses logical device numbers (LDN) to "multiplex" registers.
// The correct LDN must be set to access certain registers.
// Really just here for readability.
void Ite8786::setSioLdn(uint8_t ldn)
{
    if (ldn != m_currentLdn) {
        writeSioRegister(kLdnRegister, ldn);
        m_currentLdn = ldn;
    }
}

uint8_t Ite8786::readSioRegister(uint8_t reg)
{
    if (portio_ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    portio_outb(reg, kSpecialAddress);
    return portio_inb(kSpecialData);

    portio_ioperm(kSpecialAddress, 2, 0);
}

void Ite8786::writeSioRegister(uint8_t reg, uint8_t data)
{
    if (portio_ioperm(kSpecialAddress, 2, 1))
        throw std::system_error(
            std::make_error_code(std::errc::operation_not_permitted)
        );

    portio_outb(reg, kSpecialAddress);
    portio_outb(data, kSpecialData);

    portio_ioperm(kSpecialAddress, 2, 0);
}

uint16_t Ite8786::getChipId()
{
    uint16_t id = readSioRegister(kChipIdRegisterH) << 8;
    id |= readSioRegister(kChipIdRegisterL);
    return id;
}

uint16_t Ite8786::getBaseAddressRegister()
{
    uint16_t bar = readSioRegister(kGpioBaseRegisterH) << 8;
    bar |= readSioRegister(kGpioBaseRegisterL);
    return bar;
}