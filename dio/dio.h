#ifndef DIO_H
#define DIO_H

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

extern "C" bool init(const char* initFile);
extern "C" int digitalRead(int dio, int pin);
extern "C" int digitalWrite(int dio, int pin, bool state);
extern "C" int setOutputMode(int dio, OutputMode mode);
extern "C" const char* getLastError();

#endif