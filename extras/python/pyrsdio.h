#include <rsdio.h>

#include <memory>

/*
 * Wrapper around the RsDio class which automatically checks for errors and
 * throws them as exceptions.
 */
class PyRsDio {
   public:
    PyRsDio()
    {
        m_rsdio = std::shared_ptr<rs::RsDio>(
            rs::createRsDio(), std::mem_fn(&rs::RsDio::destroy)
        );
    }

    void setXmlFile(const char *fileName)
    {
        m_rsdio->setXmlFile(fileName);
        this->throwLastError();
    }

    bool canSetOutputMode(int dio)
    {
        bool ret = m_rsdio->canSetOutputMode(dio);
        this->throwLastError();
        return ret;
    }

    void setOutputMode(int dio, rs::OutputMode mode)
    {
        m_rsdio->setOutputMode(dio, mode);
        this->throwLastError();
    }

    rs::OutputMode getOutputMode(int dio)
    {
        rs::OutputMode mode = m_rsdio->getOutputMode(dio);
        this->throwLastError();
        return mode;
    }

    bool digitalRead(int dio, int pin)
    {
        bool state = m_rsdio->digitalRead(dio, pin);
        this->throwLastError();
        return state;
    }

    void digitalWrite(int dio, int pin, bool state)
    {
        m_rsdio->digitalWrite(dio, pin, state);
        this->throwLastError();
    }

    void setPinDirection(int dio, int pin, rs::PinDirection dir)
    {
        m_rsdio->setPinDirection(dio, pin, dir);
        this->throwLastError();
    }

    rs::PinDirection getPinDirection(int dio, int pin)
    {
        rs::PinDirection dir = m_rsdio->getPinDirection(dio, pin);
        this->throwLastError();
        return dir;
    }

    std::map<int, bool> readAll(int dio)
    {
        std::map<int, bool> states = m_rsdio->readAll(dio);
        this->throwLastError();
        return states;
    }

    rs::diomap_t getPinList() const
    {
        rs::diomap_t map = m_rsdio->getPinList();
        this->throwLastError();
        return map;
    }

    std::string getLastErrorString() const
    {
        return m_rsdio->getLastErrorString();
    }

   private:
    std::shared_ptr<rs::RsDio> m_rsdio;

    void throwLastError() const
    {
        if (std::error_code ec = m_rsdio->getLastError()) {
            throw std::system_error(ec);
        }
    }
};