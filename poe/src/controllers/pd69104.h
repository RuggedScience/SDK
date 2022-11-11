#ifndef PD69104_H
#define PD69104_H

#include "abstractpoecontroller.h"

class Pd69104 : public AbstractPoeController
{
public:
	Pd69104(uint16_t bus, uint8_t dev);
	~Pd69104() override;

	rs::PoeState getPortState(uint8_t port) override;
	void setPortState(uint8_t port, rs::PoeState state) override;

	float getPortVoltage(uint8_t port) override;
	float getPortCurrent(uint8_t port) override;

	int getBudgetConsumed() override;
	int getBudgetAvailable() override;
	int getBudgetTotal() override;

private:
	uint16_t m_busAddr;
	uint8_t m_devAddr;

	int getDeviceId() const;

	void setPortEnabled(uint8_t port, bool enabled);

	uint8_t getPortMode(uint8_t port) const;
	void setPortMode(uint8_t port, uint8_t mode);

	bool getPortSensing(uint8_t port) const;
	void setPortSensing(uint8_t port, bool sense);

	bool getPortDetection(uint8_t port) const;
	void setPortDetection(uint8_t port, bool detect);

	bool getPortClassification(uint8_t port) const;
	void setPortClassification(uint8_t port, bool classify);
};

#endif