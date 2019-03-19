#ifndef ITE8786_H
#define ITE8786_H

#include "abstractdiocontroller.h"
#include <vector>

class Ite8786 : public AbstractDioController
{
public:
	struct RegisterData
	{
		uint8_t addr;
		uint8_t ldn;
		uint8_t onBits;
		uint8_t offBits;
	};

	typedef std::vector<RegisterData> RegisterList_t;

	Ite8786();
	Ite8786(const RegisterList_t& list);
	~Ite8786();
	
	void initPin(PinInfo) override;
	PinMode getPinMode(PinInfo info) override;
	void setPinMode(PinInfo info, PinMode mode) override;

	bool getPinState(PinInfo info) override;
	void setPinState(PinInfo info, bool state) override;

private:
	uint8_t m_currentLdn;
	uint16_t m_baseAddress;

	void enterSio();
	void exitSio();

	void setSioLdn(uint8_t ldn);
	uint8_t readSioRegister(uint8_t reg);
	void writeSioRegister(uint8_t reg, uint8_t data);
	
	uint16_t getChipId();
	uint16_t getBaseAddressRegister();
};

#endif