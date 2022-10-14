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

	Ite8786(bool debug=false);
	Ite8786(const RegisterList_t& list, bool debug=false);
	~Ite8786();
	
	void initPin(PinConfig info) override;
	PinMode getPinMode(PinConfig info) override;
	void setPinMode(PinConfig info, PinMode mode) override;

	bool getPinState(PinConfig info) override;
	void setPinState(PinConfig info, bool state) override;

	void printRegs() override;

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