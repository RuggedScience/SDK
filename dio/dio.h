#ifndef DIO_H
#define DIO_H

#ifdef __linux__
#define DIO_API
#elif _WIN32
    #ifdef DIO_EXPORTS
        #define DIO_API __declspec(dllexport)
    #else
        #define DIO_API __declspec(dllimport)
    #endif //DIO_EXPORTS
#endif //_WIN32

#ifdef __cplusplus
extern "C" {
#endif

enum OutputMode
{
    ModePnp = -1,
    ModeNpn = -2    
};

enum PinMode
{
	ModeInput,
	ModeOutput
};

DIO_API bool initDio(const char* initFile);
DIO_API int digitalRead(int dio, int pin);
DIO_API int digitalWrite(int dio, int pin, bool state);
DIO_API int setOutputMode(int dio, OutputMode mode);
DIO_API const char* getLastDioError();

#ifdef __cplusplus
}
#endif
#endif