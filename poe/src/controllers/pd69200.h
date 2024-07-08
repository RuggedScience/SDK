#ifndef PD69200_H
#define PD69200_H

#include <array>
#include <chrono>

#include "abstractpoecontroller.h"

#define MSG_LEN     15
typedef std::array<uint8_t, MSG_LEN> msg_t;
typedef std::chrono::high_resolution_clock clock_timer_t;

class Pd69200 : public AbstractPoeController
{
public:
	Pd69200(uint16_t bus, uint8_t dev, uint16_t totalBudget=170);
	~Pd69200() override;

	rs::PoeState getPortState(uint8_t port) override;
	void setPortState(uint8_t port, rs::PoeState state) override;

	float getPortVoltage(uint8_t port) override;
	float getPortCurrent(uint8_t port) override;
	float getPortPower(uint8_t port) override;

	int getBudgetConsumed() override;
	int getBudgetAvailable() override;
	int getBudgetTotal() override;

private:
	uint16_t m_busAddr;
	uint8_t m_devAddr;
	uint8_t m_lastEcho;
    uint8_t m_devId;
    clock_timer_t::time_point m_lastCommandTime;

    msg_t sendMsgToController(msg_t& msg);

	uint8_t getDeviceId();

	struct PortStatus
	{
		bool enabled;
		uint8_t status;
		bool force;
		uint8_t latch;
		uint8_t classType;
		uint8_t mode; // AF / AT / POH
		bool fourPair;
	};
	PortStatus getPortStatus(uint8_t port);
	void setPortEnabled(uint8_t port, bool enable);
	void setPortForce(uint8_t port, bool force);

	struct PortMeasurements
	{
		float voltage;
		float current;
		float wattage;
	};
	PortMeasurements getPortMeasurements(uint8_t port);

	struct SystemMeasurements
	{
		int measuredWatts;
		int calculatedWatts;
		int availableWatts;
		int budgetedWatts;

	};
	SystemMeasurements getSystemMeasuerments();

	enum PowerBankSourceType
	{
		SourceTypeUnknow = 0,
		SourceTypePrimary,
		SourceTypeBackup,
		SourceTypeReserved
	};

	struct PowerBankSettings
	{
		int powerLimit;
		float maxShutdownVoltage;
		float minShutdownVoltage;
		uint8_t guardBand;
		PowerBankSourceType sourceType;
	};
	PowerBankSettings getPowerBankSettings(uint8_t bank);
	void setPowerBankSettings(uint8_t bank, const PowerBankSettings &settings);
};

#endif // PD69200_H