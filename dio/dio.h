#ifndef DIO_H
#define DIO_H

#include <map>
#include <string>

class AbstractDioController;

class Dio
{
public:
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

    struct PinInfo
	{
		uint8_t bitmask;
		uint8_t offset;
		bool invert;
		bool supportsInput;
		bool supportsOutput; 

		PinInfo() :
			bitmask(0),
			offset(0),
			invert(0),
			supportsInput(false),
			supportsOutput(false)
		{}

		PinInfo(uint8_t bit, uint8_t gpio, bool inv, bool input, bool output) :
			bitmask(1 << bit),
			offset(gpio - 1),
			invert(inv),
			supportsInput(input),
			supportsOutput(output)
		{}
	};

    Dio();
    Dio(const char *initFile);
    Dio(std::string initFile);
    ~Dio();

    bool open();
    bool open(const char *initFile);
    bool open(std::string initFile);

    int digitalRead(int dio, int pin);
    int digitalWrite(int dio, int pin, bool state);

    int setOutputMode(int dio, OutputMode mode);

    std::string getLastError();

private:
    std::string m_initFile;
    std::string m_lastError;
    AbstractDioController *mp_controller;

    typedef std::map<int, PinInfo> pinmap_t;
    std::map<int, pinmap_t> m_dioMap;
};

#endif