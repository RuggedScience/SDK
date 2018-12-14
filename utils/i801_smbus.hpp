#ifndef SMBUS_HPP
#define SMBUS_HPP

#include <stdint.h>
#include <stdexcept>

#ifdef __linux__
#include <sys/io.h>
#elif _WIN32
#include "portio.hpp"
#endif

static const int kMaxRetry = 400;

//SMBus Registers and bits as described in the Intel chipset datasheet. (Page 746 Table 18-2)
//https://www.intel.com/content/dam/www/public/us/en/documents/datasheets/6-chipset-c200-chipset-datasheet.pdf
static const uint8_t kStsDone		= (1 << 7);
static const uint8_t kStsInUse		= (1 << 6);
static const uint8_t kStsAlert		= (1 << 5);
static const uint8_t kStsFailed		= (1 << 4);
static const uint8_t kStsBusErr		= (1 << 3);
static const uint8_t kStsDevErr		= (1 << 2);
static const uint8_t kStsIntr		= (1 << 1);
static const uint8_t kStsBusy		= (1 << 0);

//Bits 2-4 are for the command
static const uint8_t kCntrlPecEn	= (1 << 7);
static const uint8_t kCntrlStart	= (1 << 6);
static const uint8_t kCntrlLastByte = (1 << 5);
static const uint8_t kCntrlKill		= (1 << 1);
static const uint8_t kCntrlIntrEn	= (1 << 0);

static const uint8_t kCntrlByteCmd	= 0b01000;

#define HST_STS(x) 	x
#define HST_CTRL(x) 	(x + 2)
#define HST_CMD(x) 	(x + 3)
#define HST_XMIT(x)	(x + 4)
#define HST_DATA0(x)	(x + 5)

//Added both of the below functions for readability.
static void clearStatusBits(uint16_t bus, uint8_t bits)
{
	outb(bits, HST_STS(bus)); //Must write bits high to clear flags in register
}

static bool isBitSet(uint8_t val, uint8_t bits)
{
	return ((val & bits) != 0);
}

// Function supplied by manufacturer. Only altered for readability.
int smbusReadRegister(uint16_t bus, uint8_t dev, uint8_t reg)
{
	if (ioperm(bus, 6, 1))
	{
		errno = EACCES;
		return -1;
	}

	uint8_t status = inb(HST_STS(bus));
	if (isBitSet(status, kStsDone | kStsFailed))
		outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));

	clearStatusBits(bus, 0xFF);
	outb(0x00, HST_DATA0(bus));

	int i;
	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));
		if (isBitSet(status, kStsDone | kStsFailed))
			outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));

		if (isBitSet(status, kStsDone | kStsFailed | kStsDevErr))
			clearStatusBits(bus, status | kStsDone | kStsFailed | kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 6, 0);
		errno = EBUSY;
		return -1;
	}

	outb(dev + 1, HST_XMIT(bus));
	outb(reg, HST_CMD(bus));
	outb(kCntrlStart | kCntrlByteCmd, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));
		if (isBitSet(status, kStsDevErr)) clearStatusBits(bus, kStsDevErr);
		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 6, 0);
		errno = EBUSY;
		return -1;
	}

	uint8_t data = inb(HST_DATA0(bus));
	ioperm(bus, 6, 0);
	return data;
}

// Function supplied by manufacturer. Only altered for readability.
int smbusWriteRegister(uint16_t bus, uint8_t dev, uint8_t reg, uint8_t val)
{
	if (ioperm(bus, 6, 1))
	{
		errno = EACCES;
		return -1;
	}

	uint8_t status = inb(HST_STS(bus));
	if (isBitSet(status, kStsDone | kStsFailed))
		outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));

	clearStatusBits(bus, 0xFF);

	int i;
	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));
		if (isBitSet(status, kStsDone | kStsFailed))
			outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));
			
		if (isBitSet(status, kStsDone | kStsFailed | kStsDevErr))
			clearStatusBits(bus, status | kStsDone | kStsFailed | kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 6, 0);
		errno = EBUSY;
		return -1;
	}

	outb(dev, HST_XMIT(bus));
	outb(reg, HST_CMD(bus));
	outb(val, HST_DATA0(bus));
	outb(kCntrlStart | kCntrlByteCmd, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));
		if (isBitSet(status, kStsDevErr)) clearStatusBits(bus, kStsDevErr);
		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 6, 0);
		errno = EBUSY;
		return -1;
	}

	ioperm(bus, 6, 0);
	return 0;
}

#endif //SMBUS_HPP