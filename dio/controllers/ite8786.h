#ifndef ITE8786_H
#define ITE8786_H

#include "abstractdiocontroller.h"

class Ite8786 : public AbstractDioController
{
public:
	Ite8786();
	~Ite8786();
	
	void initPin(Dio::PinInfo) override;
	Dio::PinMode getPinMode(Dio::PinInfo info) override;
	void setPinMode(Dio::PinInfo info, Dio::PinMode mode) override;

	bool getPinState(Dio::PinInfo info) override;
	void setPinState(Dio::PinInfo info, bool state) override;

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