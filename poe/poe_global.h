#ifndef POE_GLOBAL_H
#define POE_GLOBAL_H

enum PoeState
{
    StateDisabled,		//No power regardless of attached device.
    StateEnabled,		//Power always applied regardless of attached device.
    StateAuto,			//Normal PoE operation.
    StateError          //Error retreiving state. Use getLastError and getLastErrorString for more details.
};

#endif //POE_GLOBAL_H