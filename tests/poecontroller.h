#include <map>
#include <vector>

#include "../poe/src/controllers/abstractpoecontroller.h"

struct PortStatus {
    rs::PoeState state;
    float voltage;
    float current;
};

class TestPoeController : public AbstractPoeController {
   public:
    TestPoeController(int budget, std::vector<uint8_t> ports)
        : m_budgetTotal(budget)
    {
        for (auto port : ports) {
            PortStatus status;
            status.state = rs::PoeState::Auto;
            status.voltage = 0;
            status.current = 0;

            m_ports[port] = status;
        }
    }

    rs::PoeState getPortState(uint8_t port) override final
    {
        return getOrThrow(port).state;
    }

    void setPortState(uint8_t port, rs::PoeState state) override final
    {
        getOrThrow(port).state = state;
    }

    float getPortVoltage(uint8_t port) override final
    {
        return getOrThrow(port).voltage;
    }

    void setPortVoltage(uint8_t port, float voltage)
    {
        getOrThrow(port).voltage = voltage;
    }

    float getPortCurrent(uint8_t port) override final
    {
        return getOrThrow(port).current;
    }

    float setPortCurrent(uint8_t port, float current)
    {
        return getOrThrow(port).current = current;
    }

    int getBudgetConsumed() override final { return m_budgetConsumed; }

    int getBudgetAvailable() override final
    {
        return m_budgetTotal - m_budgetConsumed;
    }

    int getBudgetTotal() override final { return m_budgetTotal; }

   private:
    PortStatus &getOrThrow(uint8_t port)
    {
        auto it = m_ports.find(port);
        if (it == m_ports.end()) {
            throw std::system_error(
                std::make_error_code(std::errc::invalid_argument),
                "Invalid port"
            );
        }

        return it->second;
    }

    int m_budgetTotal;
    int m_budgetConsumed;
    std::map<uint8_t, PortStatus> m_ports;
};