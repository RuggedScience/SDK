#ifndef ITE8786_H
#define ITE8786_H

#include "abstractdiocontroller.h"
#include <vector>

class Ite8786 : public AbstractDioController
{
public:
	Ite8786();
	Ite8786(const std::vector<RegisterConfig> &registers);
	~Ite8786();
	
	void initPin(const DioPinConfig &config) override;
	rs::PinDirection getPinMode(const DioPinConfig &config) override;
	void setPinMode(const DioPinConfig &config, rs::PinDirection mode) override;

	bool getPinState(const DioPinConfig &config) override;
	void setPinState(const DioPinConfig &config, bool state) override;

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