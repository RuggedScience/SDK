#include <assert.h>

#include <iostream>
#include <system_error>

#include "i801_smbus.h"

#ifdef __linux__
#include <sys/io.h>
#elif _WIN32
#include "portio.hpp"
#endif

static const int kMaxRetry = 10000;

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

static const uint8_t kCntrlQuick	= 0x00;
static const uint8_t kCntrlByte		= 0x04;
static const uint8_t kCntrlByteData	= 0x08;
static const uint8_t kCntrlWordData = 0x0C;
static const uint8_t kCntrlProcCall	= 0x10;
static const uint8_t kCntrlBlck		= 0x14;
static const uint8_t kCntrlI2CRead	= 0x18;
static const uint8_t kCntrlBlckProc	= 0x1C;

static const uint8_t kCntrlByteCmd	= 0b01000;

#define HST_STS(x) 		x
#define HST_CTRL(x) 	(x + 2)
#define HST_CMD(x) 		(x + 3)
#define HST_XMIT(x)		(x + 4)
#define HST_DATA0(x)	(x + 5)
#define HST_DATA1(x)	(x + 6)
#define HST_BLK_DB(x)	(x + 7)

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
uint8_t smbusReadRegister(uint16_t bus, uint8_t dev, uint8_t reg)
{
	if (ioperm(bus, 8, 1))
		throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));

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

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	outb(dev + 1, HST_XMIT(bus));
	outb(reg, HST_CMD(bus));
	outb(kCntrlStart | kCntrlByteData, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));

		if (isBitSet(status, kStsDevErr)) 
			clearStatusBits(bus, kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	uint8_t data = inb(HST_DATA0(bus));
	ioperm(bus, 8, 0);
	return data;
}

// Function supplied by manufacturer. Only altered for readability.
void smbusWriteRegister(uint16_t bus, uint8_t dev, uint8_t reg, uint8_t val)
{
	if (ioperm(bus, 8, 1))
		throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));

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

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	outb(dev, HST_XMIT(bus));
	outb(reg, HST_CMD(bus));
	outb(val, HST_DATA0(bus));
	outb(kCntrlStart | kCntrlByteData, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));

		if (isBitSet(status, kStsDevErr)) 
			clearStatusBits(bus, kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) 
			break;
	}

	ioperm(bus, 8, 0);
	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)	
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
}

// Function supplied by manufacturer. Only altered for readability.
uint8_t smbusReadByte(uint16_t bus, uint8_t dev)
{
	if (ioperm(bus, 8, 1))
		throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));

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

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	outb(dev + 1, HST_XMIT(bus));
	outb(kCntrlStart | kCntrlByteData, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));

		if (isBitSet(status, kStsDevErr)) 
			clearStatusBits(bus, kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	uint8_t data = inb(HST_CMD(bus));
	ioperm(bus, 8, 0);
	return data;
}

void smbusWriteByte(uint16_t bus, uint8_t dev, uint8_t val)
{
	if (ioperm(bus, 8, 1))
		throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));

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

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	outb(dev, HST_XMIT(bus));
	outb(val, HST_CMD(bus));
	outb(kCntrlStart | kCntrlByte, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));

		if (isBitSet(status, kStsDevErr)) 
			clearStatusBits(bus, kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == (kStsInUse | kStsIntr)) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	ioperm(bus, 8, 0);
}

void smbusI2CRead(uint16_t bus, uint8_t dev, uint8_t cmd, uint8_t *buf, size_t size)
{
	uint16_t i, j, k;

	assert(buf != nullptr);
	assert(size > 0);

	if (ioperm(bus, 8, 1))
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));

	uint8_t status = inb(HST_STS(bus));
	if (isBitSet(status, kStsDone | kStsFailed))
		outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));

	clearStatusBits(bus, 0xFF);
	outb(0x00, HST_DATA0(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));
		if (isBitSet(status, kStsDone | kStsFailed))
			outb(inb(HST_CTRL(bus)) | (status & kStsDone) | kCntrlKill, HST_CTRL(bus));
			
		if (isBitSet(status, kStsDone | kStsFailed | kStsDevErr))
			clearStatusBits(bus, status | kStsDone | kStsFailed | kStsDevErr);

		if (isBitSet(status, kStsDevErr) || status == kStsInUse) 
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	outb(dev, HST_XMIT(bus));
	outb(cmd, HST_DATA1(bus));
	outb(kCntrlStart | kCntrlI2CRead, HST_CTRL(bus));

	for (i = 0; i < kMaxRetry; ++i)
	{
		status = inb(HST_STS(bus));

		if (isBitSet(status, kStsDevErr)) 
			clearStatusBits(bus, kStsDevErr);

		if (isBitSet(status, kStsDevErr) || ((status & 0xC0) == 0xC0 && status & 0x3))
			break;
	}

	if (isBitSet(status, kStsDevErr) || i >= kMaxRetry)
	{
		ioperm(bus, 8, 0);
		throw std::system_error(std::make_error_code(std::errc::device_or_resource_busy));
	}

	uint8_t data = 0;
	for (j = 0, k = 0; i <= kMaxRetry; )
	{
		data = inb(bus + 0x07);
		outb(0xFF, HST_STS(bus));
		for (; i <= kMaxRetry; i++)
		{
			status = inb(HST_STS(bus));

			if (isBitSet(status, kStsDevErr)) 
				clearStatusBits(bus, kStsDevErr);

			if (isBitSet(status, kStsDevErr) || ((status & 0xC0) == 0xC0 && status & 0x3))
				break;
		}

		if (j && !(j % size) && !data)
		{
			k++;
			if (i > kMaxRetry || k > size + 1)
			{
				outb(0x3A, bus + 0x02);
				break;
			}
		}
		else k = 0;

		if (data || j % size)
		{
			buf[j % size] = data;
			j++;
		}
	}

	if (!j || j % size || data || (i <= kMaxRetry && k <= size + 1))
		outb(0x3A, bus + 0x02);

	ioperm(bus, 8, 0);
}