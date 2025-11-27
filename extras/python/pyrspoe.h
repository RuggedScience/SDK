#include <rspoe.h>

#include <memory>

/*
 * Wrapper around the RsPoe class which automatically checks for errors and
 * throws them as exceptions.
 */
class PyRsPoe {
   public:
    PyRsPoe()
    {
        m_rspoe = std::shared_ptr<rs::RsPoe>(
            rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy)
        );
    }

    void init(const char *configFile)
    {
        m_rspoe->init(configFile);
    }

    std::vector<int> getPortList() const { return m_rspoe->getPortList(); }

    rs::PoeState getPortState(int port)
    {
        return m_rspoe->getPortState(port);
    }

    void setPortState(int port, rs::PoeState state)
    {
        m_rspoe->setPortState(port, state);
    }

    float getPortVoltage(int port)
    {
        return m_rspoe->getPortVoltage(port);
    }

    float getPortCurrent(int port)
    {
        return m_rspoe->getPortCurrent(port);
    }

    float getPortPower(int port)
    {
        return m_rspoe->getPortPower(port);
    }

    int getBudgetConsumed()
    {
        return m_rspoe->getBudgetConsumed();
    }

    int getBudgetAvailable()
    {
        return m_rspoe->getBudgetAvailable();
    }

    int getBudgetTotal()
    {
        return m_rspoe->getBudgetTotal();
    }

   private:
    std::shared_ptr<rs::RsPoe> m_rspoe;
};