#ifndef POE_H
#define POE_H

enum PoeState
{
    StateDisabled,		//No power regardless of attached device.
    StateEnabled,		//Power always applied regardless of attached device.
    StateAuto,			//Normal PoE operation.
    StateError          //Error retreiving state. Use getLastError and getLastErrorString for more details.
};

extern "C" bool init(const char *initFile);
extern "C" PoeState getPortState(int port);
extern "C" int setPortState(int port, PoeState state);
extern "C" const char* getLastError();

#endif