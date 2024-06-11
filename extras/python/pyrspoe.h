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

    void setXmlFile(const char *fileName)
    {
        m_rspoe->setXmlFile(fileName);
        this->throwLastError();
    }

    std::vector<int> getPortList() const { return m_rspoe->getPortList(); }

    rs::PoeState getPortState(int port)
    {
        rs::PoeState ret = m_rspoe->getPortState(port);
        this->throwLastError();
        return ret;
    }

    void setPortState(int port, rs::PoeState state)
    {
        m_rspoe->setPortState(port, state);
        this->throwLastError();
    }

    float getPortVoltage(int port)
    {
        float ret = m_rspoe->getPortVoltage(port);
        this->throwLastError();
        return ret;
    }

    float getPortCurrent(int port)
    {
        float ret = m_rspoe->getPortCurrent(port);
        this->throwLastError();
        return ret;
    }

    float getPortPower(int port)
    {
        float ret = m_rspoe->getPortPower(port);
        this->throwLastError();
        return ret;
    }

    int getBudgetConsumed()
    {
        int ret = m_rspoe->getBudgetConsumed();
        this->throwLastError();
        return ret;
    }

    int getBudgetAvailable()
    {
        int ret = m_rspoe->getBudgetAvailable();
        this->throwLastError();
        return ret;
    }

    int getBudgetTotal()
    {
        int ret = m_rspoe->getBudgetTotal();
        this->throwLastError();
        return ret;
    }

   private:
    std::shared_ptr<rs::RsPoe> m_rspoe;

    void throwLastError() const
    {
        if (std::error_code ec = m_rspoe->getLastError()) {
            throw std::system_error(ec);
        }
    }
};