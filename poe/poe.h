#ifndef POE_H
#define POE_H

#ifdef __linux__
#define POE_API
#elif _WIN32
    #ifdef POE_EXPORTS
        #define POE_API __declspec(dllexport)
    #else
        #define POE_API __declspec(dllimport)
    #endif //POE_EXPORTS
#endif //_WIN32

#ifdef __cplusplus
extern "C" {
#endif

enum PoeState
{
    StateDisabled,		//No power regardless of attached device.
    StateEnabled,		//Power always applied regardless of attached device.
    StateAuto,			//Normal PoE operation.
    StateError          //Error retreiving state. Use getLastError and getLastErrorString for more details.
};

POE_API bool initPoe(const char *initFile);
POE_API PoeState getPortState(int port);
POE_API int setPortState(int port, PoeState state);
POE_API const char* getLastPoeError();

#ifdef __cplusplus
}
#endif
#endif