#include "i801_smbus.h"

#include <assert.h>

#include <chrono>
#include <iostream>
#include <system_error>
#include <thread>

#ifdef __linux__
#include <sys/io.h>
#elif _WIN32
#include "portio.hpp"
#endif

#define BIT(x) (1 << x)
#define SMBUS_READ 1
#define SMBUS_WRITE 0
#define SMBUS_MAX_BLOCK_SIZE 32
#define SMBUS_LEN_SENTINEL SMBUS_MAX_BLOCK_SIZE + 1

typedef std::chrono::nanoseconds nanoseconds_t;
typedef std::chrono::microseconds microseconds_t;
typedef std::chrono::milliseconds milliseconds_t;

typedef std::chrono::high_resolution_clock transaction_clock_t;
typedef transaction_clock_t::time_point time_point_t;

static const nanoseconds_t sleep_time{10};
static const milliseconds_t max_time{100};
static time_point_t transaction_start;

// SMBus Registers and bits as described in the Intel chipset datasheet. (Page
// 746 Table 18-2)
// https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/6-chipset-c200-chipset-datasheet.pdf
static const uint8_t kStsDone = BIT(7);
static const uint8_t kStsInUse = BIT(6);
static const uint8_t kStsAlert = BIT(5);
static const uint8_t kStsFailed = BIT(4);
static const uint8_t kStsBusErr = BIT(3);
static const uint8_t kStsDevErr = BIT(2);
static const uint8_t kStsIntr = BIT(1);
static const uint8_t kStsBusy = BIT(0);

static const uint8_t kStsErrorFlags = (kStsFailed | kStsBusErr | kStsDevErr);
static const uint8_t kStsFlags = (kStsDone | kStsIntr | kStsErrorFlags);

// Bits 2-4 are for the command
static const uint8_t kCntrlPecEn = BIT(7);
static const uint8_t kCntrlStart = BIT(6);
static const uint8_t kCntrlLastByte = BIT(5);
static const uint8_t kCntrlKill = BIT(1);
static const uint8_t kCntrlIntrEn = BIT(0);

static const uint8_t kCntrlQuick = 0x00;
static const uint8_t kCntrlByte = 0x04;
static const uint8_t kCntrlByteData = 0x08;
static const uint8_t kCntrlWordData = 0x0C;
static const uint8_t kCntrlProcCall = 0x10;
static const uint8_t kCntrlBlck = 0x14;
static const uint8_t kCntrlI2CRead = 0x18;
static const uint8_t kCntrlBlckProc = 0x1C;

static const uint8_t kAuxCntrlE32b = BIT(1);
static const uint8_t kAuxCntrlCrc = BIT(0);

#define HST_STS(x) (x + 0x0)
#define HST_CTRL(x) (x + 0x2)
#define HST_CMD(x) (x + 0x3)
#define HST_XMIT(x) (x + 0x4)
#define HST_DATA0(x) (x + 0x5)
#define HST_DATA1(x) (x + 0x6)
#define HST_BLK_DB(x) (x + 0x7)
#define AUX_CTL(x) (x + 0xD)
#define IO_MAX 0x17

enum class transaction_type {
    QUICK = 0x00,
    BYTE = 0x04,
    BYTE_DATA = 0x08,
    WORD_DATA = 0x0C,
    PROC_CALL = 0x10,
    BLOCK = 0x14,
    I2C_READ = 0x18,
    BLOCK_PROC = 0x1C
};

struct transaction_data {
    uint16_t bus;
    uint8_t device;
    transaction_type type;
    uint8_t command;
    uint8_t *block;
    uint8_t size;
    char read_write;
};

static milliseconds_t timeElapsed()
{
    const time_point_t now = transaction_clock_t::now();
    return std::chrono::duration_cast<milliseconds_t>(now - transaction_start);
}

static milliseconds_t timeLeft()
{
    return std::chrono::duration_cast<milliseconds_t>(max_time - timeElapsed());
}

static bool isBlockTransaction(transaction_data *data)
{
    return (
        data->type == transaction_type::BLOCK ||
        data->type == transaction_type::I2C_READ
    );
}

static int initBus(transaction_data *data)
{
    if (ioperm(data->bus, IO_MAX, 1)) {
        return -EPERM;
    }

    int status = inb(HST_STS(data->bus));
    if (status & kStsBusy) {
        return -EBUSY;
    }

    status &= kStsFlags;
    if (status) {
        // Clear flags
        outb(status, HST_STS(data->bus));
    }

    status = inb(HST_STS(data->bus));

    // Disable CRC / PEC
    // outb(inb(AUX_CTL(bus)) & (~kAuxCntrlCrc), AUX_CTL(bus));

    return 0;
}

static void cleanupBus(transaction_data *data)
{
    outb(kStsInUse | kStsFlags, HST_STS(data->bus));
    ioperm(data->bus, IO_MAX, 0);
}

static void setHostAddress(transaction_data *data)
{
    // Assume we are given the 7-bit address instead of the 8-bit address.
    outb((data->device << 0) | (data->read_write & 0x01), HST_XMIT(data->bus));
}

static int waitForIntr(transaction_data *data)
{
    int status, busy;

    do {
        std::this_thread::sleep_for(sleep_time);

        status = inb(HST_STS(data->bus));
        busy = status & kStsBusy;
        status &= kStsErrorFlags | kStsIntr;
        if (!busy && status) return status & kStsErrorFlags;

    } while (timeLeft().count() > 0);

    return -ETIMEDOUT;
}

static int waitForByteDone(transaction_data *data)
{
    int status;

    do {
        std::this_thread::sleep_for(sleep_time);
        status = inb(HST_STS(data->bus));
        if (status & (kStsErrorFlags | kStsDone))
            return status & kStsErrorFlags;
    } while (timeLeft().count() > 0);

    return -ETIMEDOUT;
}

static void startTransaction(transaction_data *data)
{
    uint8_t ctrl = (uint8_t)data->type | kCntrlStart;

    // Block read transaction that's only one byte.
    // Need to set the last byte flag.
    if (isBlockTransaction(data) && data->read_write == SMBUS_READ &&
        data->size == 1) {
        ctrl |= kCntrlLastByte;
    }

    transaction_start = transaction_clock_t::now();
    outb(ctrl, HST_CTRL(data->bus));
}

static void throwError(std::errc error)
{
    throw std::system_error(std::make_error_code(error));
}

static void throwError(std::errc error, const char *message)
{
    throw std::system_error(std::make_error_code(error), message);
}

static void handleResult(transaction_data *data, int status)
{
    // Positive error codes indicate an error from the bus
    // which means the transaction should already be terminated.
    // Negative error codes indicate an error in this library (not reported from the bus)
    // so we need to make sure we kill the transaction.
    if (status < 0) {
        // Try to kill the current command
        outb(kCntrlKill, HST_CTRL(data->bus));
        std::this_thread::sleep_for(sleep_time);
        outb(0, HST_CTRL(data->bus));
    }

    if (status) {
        cleanupBus(data);
        if (status == -ETIMEDOUT) {
            throwError(std::errc::timed_out);
        }
        else if (status == -EBUSY) {
            throwError(std::errc::device_or_resource_busy);
        }
        else if (status == -EPROTO) {
            throwError(std::errc::protocol_error);
        }
        else if (status == kStsFailed) {
            throwError(std::errc::io_error);
        }
        else if (status == kStsDevErr) {
            throwError(std::errc::no_such_device_or_address);
        }
        else if (status == kStsBusErr) {
            throwError(std::errc::resource_unavailable_try_again);
        } else {
            throw std::runtime_error("Unknown SMBus Error");
        }
    }
}

static void smbus_transaction(transaction_data *data)
{
    initBus(data);
    setHostAddress(data);

    switch (data->type) {
        case transaction_type::QUICK:
            break;
        case transaction_type::BYTE:
            if (data->read_write == SMBUS_WRITE)
                outb(data->command, HST_CMD(data->bus));
            break;
        case transaction_type::BYTE_DATA:
            if (data->read_write == SMBUS_WRITE)
                outb(data->block[0], HST_DATA0(data->bus));
            outb(data->command, HST_CMD(data->bus));
            break;
        case transaction_type::WORD_DATA:
            if (data->read_write == SMBUS_WRITE) {
                outb(data->block[0], HST_DATA0(data->bus));
                outb(data->block[1], HST_DATA1(data->bus));
            }
            break;
        case transaction_type::BLOCK:
            outb(data->command, HST_CMD(data->bus));
            if (data->read_write == SMBUS_WRITE) {
                outb(data->size, HST_DATA0(data->bus));
                outb(data->block[0], HST_BLK_DB(data->bus));
            }
            break;
        case transaction_type::I2C_READ:
            outb(data->command, HST_DATA1(data->bus));
            outb(0x00, HST_BLK_DB(data->bus));
            break;
        default:
            throwError(std::errc::not_supported);
    }

    startTransaction(data);

    if (!isBlockTransaction(data)) {
        handleResult(data, waitForIntr(data));
        switch (data->type) {
            case transaction_type::WORD_DATA:
                if (data->read_write == SMBUS_READ)
                    data->block[1] = inb(HST_DATA1(data->bus));
            case transaction_type::BYTE:
            case transaction_type::BYTE_DATA:
                if (data->read_write == SMBUS_READ)
                    data->block[0] = inb(HST_DATA0(data->bus));
        }
    }
    else {
        for (size_t i = 0; i < data->size; i++) {
            handleResult(data, waitForByteDone(data));

            if (data->read_write == SMBUS_READ) {
                // Read transactions need to get the size from the device.
                if (data->size == SMBUS_LEN_SENTINEL) {
                    data->size = inb(HST_DATA0(data->bus));
                    if (data->size < 1 || data->size > SMBUS_MAX_BLOCK_SIZE) {
                        handleResult(data, -EPROTO);
                    }
                }

                data->block[i] = inb(HST_BLK_DB(data->bus));
                // If next read is our last byte, we need to inform the PCH.
                if (i + 1 == data->size)
                    outb(
                        (uint8_t)data->type | kCntrlLastByte,
                        HST_CTRL(data->bus)
                    );
            }
            else {
                outb(data->block[i], HST_BLK_DB(data->bus));
            }

            outb(kStsDone, HST_STS(data->bus));
        }
    }

    cleanupBus(data);
}

uint8_t smbus_read(uint16_t bus, uint8_t device)
{
    uint8_t block[1];
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::BYTE;
    data.block = block;
    data.size = 1;
    data.read_write = SMBUS_READ;
    smbus_transaction(&data);
    return data.block[0];
}

void smbus_write(uint16_t bus, uint8_t device, uint8_t command)
{
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::BYTE;
    data.read_write = SMBUS_WRITE;
    data.command = command;
    smbus_transaction(&data);
}

uint8_t smbus_read_register(uint16_t bus, uint8_t device, uint8_t command)
{
    uint8_t block[1];
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::BYTE_DATA;
    data.command = command;
    data.block = block;
    data.size = 1;
    data.read_write = SMBUS_READ;

    smbus_transaction(&data);
    return block[0];
}

void smbus_write_register(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t value
)
{
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.command = command;
    data.block = &value;
    data.size = 1;
    data.read_write = SMBUS_WRITE;
    data.type = transaction_type::BYTE_DATA;
    smbus_transaction(&data);
}

void smbus_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t *block
)
{
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::BLOCK;
    data.block = block;
    data.size = SMBUS_LEN_SENTINEL;
    data.read_write = SMBUS_READ;
    smbus_transaction(&data);
}

void smbus_write_block(
    uint16_t bus,
    uint8_t device,
    uint8_t *block,
    uint8_t size
)
{
    if (size > SMBUS_MAX_BLOCK_SIZE) {
        throwError(std::errc::protocol_error, "Invalid i2c read size");
    }
    
    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::BLOCK;
    data.block = block;
    data.size = size;
    data.read_write = SMBUS_WRITE;
    smbus_transaction(&data);
}

void i2c_read_block(
    uint16_t bus,
    uint8_t device,
    uint8_t command,
    uint8_t *buf,
    uint8_t size
)
{
    if (size > SMBUS_MAX_BLOCK_SIZE) {
        throwError(std::errc::protocol_error, "Invalid i2c read size");
    }

    transaction_data data;
    data.bus = bus;
    data.device = device;
    data.type = transaction_type::I2C_READ;
    data.block = buf;
    data.size = size;
    data.read_write = SMBUS_READ;
    smbus_transaction(&data);
}