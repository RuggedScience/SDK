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

    Poe();
    Poe(const char *initFile);
    Poe(std::string initFile);
    ~Poe();

    bool open();
    bool open(const char *initFile);
    bool open(std::string initFile);
    
    PoeState getPortState(int port);
    int setPortState(int port, PoeState state);

    std::string getLastError();

private:
    std::string m_initFile;
    std::string m_lastError;
    AbstractPoeController *mp_controller;
    std::map<int, uint8_t> m_portMap;

};

#endif