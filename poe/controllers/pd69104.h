#ifndef PD69104_H
#define PD69104_H

#include "abstractpoecontroller.h"

class Pd69104 : public AbstractPoeController
{
public:
	Pd69104(uint16_t bus, uint8_t dev);
	~Pd69104() override;

	Poe::PoeState getPortState(uint8_t port) override;
	void setPortState(uint8_t port, Poe::PoeState state) override;

private:
	uint16_t m_busAddr;
	uint8_t m_devAddr;

	int getDeviceId();

	void setPortEnabled(uint8_t port, bool enabled);

	uint8_t getPortMode(uint8_t port);
	void setPortMode(uint8_t port, uint8_t mode);

	bool getPortSensing(uint8_t port);
	void setPortSensing(uint8_t port, bool sense);

	bool getPortDetection(uint8_t port);
	void setPortDetection(uint8_t port, bool detect);

	bool getPortClassification(uint8_t port);
	void setPortClassification(uint8_t port, bool classify);
};

#endif