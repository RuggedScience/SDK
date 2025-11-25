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
	
	void initPin(const PinConfig &config) override;
	PinMode getPinMode(const PinConfig &config) override;
	void setPinMode(const PinConfig &config, PinMode mode) override;

	bool getPinState(const PinConfig &config) override;
	void setPinState(const PinConfig &config, bool state) override;

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