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

    void init(const char *configFile)
    {
        m_rsdio->init(configFile);
    }

    bool canSetOutputMode(int dio)
    {
        return m_rsdio->canSetOutputMode(dio);
    }

    void setOutputMode(int dio, rs::OutputMode mode)
    {
        m_rsdio->setOutputMode(dio, mode);
    }

    rs::OutputMode getOutputMode(int dio)
    {
        return m_rsdio->getOutputMode(dio);
    }

    bool digitalRead(int dio, int pin)
    {
        return m_rsdio->digitalRead(dio, pin);
    }

    void digitalWrite(int dio, int pin, bool state)
    {
        m_rsdio->digitalWrite(dio, pin, state);
    }

    void setPinDirection(int dio, int pin, rs::PinDirection dir)
    {
        m_rsdio->setPinDirection(dio, pin, dir);
    }

    rs::PinDirection getPinDirection(int dio, int pin)
    {
        return m_rsdio->getPinDirection(dio, pin);
    }

    std::map<int, bool> readAll(int dio)
    {
        return m_rsdio->readAll(dio);
    }

    rs::diomap_t getPinList() const
    {
        return m_rsdio->getPinList();
    }

   private:
    std::shared_ptr<rs::RsDio> m_rsdio;
};