#ifndef ITE8783_H
#define ITE8783_H

#include "abstractdiocontroller.h"

class Ite8783 : public AbstractDioController
{
public:
	Ite8783(bool debug=false);
	~Ite8783();
	
	void initPin(PinConfig info) override;
	PinMode getPinMode(PinConfig info) override;
	void setPinMode(PinConfig info, PinMode mode) override;

	bool getPinState(PinConfig info) override;
	void setPinState(PinConfig info, bool state) override;

	void printRegs() override;

private:
	uint16_t m_baseAddress;

	void enterSio();
	void exitSio();
	uint8_t readSioRegister(uint8_t reg);
	void writeSioRegister(uint8_t reg, uint8_t data);
	void setSioLdn(uint8_t ldn);
	uint16_t getChipId();
	uint16_t getBaseAddressRegister();
};
#endif