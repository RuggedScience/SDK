#ifndef POE_H
#define POE_H

#include <map>
#include <string>
#include <stdint.h>

class AbstractPoeController;

class Poe
{
public:
    enum PoeState
    {
    	StateDisabled,		//No power regardless of attached device.
    	StateEnabled,		//Power always applied regardless of attached device.
    	StateAuto,			//Normal PoE operation.
        StateError          //Error retreiving state. Use getLastError and getLastErrorString for more details.
    };

    enum PoeError
    {
        NoError,
        XmlError,           //Error parsing the provided XML file.
        PortError,          //Requested port does not exist.
        ControllerError     //Error while communicating with the PoE controller
    };

    Poe();
    Poe(const char *initFile);
    Poe(std::string initFile);
    ~Poe();

    bool open();
    bool open(const char *initFile);
    bool open(std::string initFile);
    
    PoeState getPortState(int port);
    bool setPortState(int port, PoeState state);

    PoeError getLastError();
    std::string getLastErrorString();

private:
    std::string m_initFile;
    PoeError m_lastError;
    std::string m_lastErrorString;
    AbstractPoeController *mp_controller;
    std::map<int, uint8_t> m_portMap;

    void setLastError(PoeError error, const char *string);
    void setLastError(PoeError error, std::string string);
};

#endif