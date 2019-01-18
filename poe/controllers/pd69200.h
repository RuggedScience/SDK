#ifndef PD69200_H
#define PD69200_H

#include "abstractpoecontroller.h"

class Pd69200 : public AbstractPoeController
{
public:
	Pd69200(uint16_t bus, uint8_t dev);
	~Pd69200() override;

	PoeState getPortState(uint8_t port) override;
	void setPortState(uint8_t port, PoeState state) override;

	float getPortVoltage(uint8_t port) override;
	float getPortCurrent(uint8_t port) override;

private:
	uint16_t m_busAddr;
	uint8_t m_devAddr;
	uint8_t m_lastEcho;

	void sendMsgToController(std::vector<uint8_t>& msg, bool response = false);

	int getDeviceId();

	bool getPortEnabled(uint8_t port);
	bool getPortForce(uint8_t port);
};

#endif // PD69200_H