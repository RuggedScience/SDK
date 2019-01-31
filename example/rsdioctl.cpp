#include "../dio/rsdio.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

static int stringToState(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	if (str == "DISABLED" || str == "FALSE" || str == "0")
		return 0;
	else if (str == "ENABLED" || str == "TRUE" || str == "1")
		return 1;
	
	return -1;
}

static const char *stateToString(bool state)
{
	if (!state)
		return "disabled";
	else
		return "enabled";
}

static OutputMode stringToMode(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	if (str == "PNP" || str == "0")
		return ModePnp;
	else if (str == "NPN" || str == "1")
		return ModeNpn;

	return ModeError;
}

void printLastError()
{
	printf("Error: %s\n", getLastDioError());
}

static void showUsage()
{
	std::cout 	<< "Usage: rsdioctl FILE COMMAND [OPTIONS...]\n"
				<< "\n"
				<< "Commands:\n"
				<< "s, state\t\toutput the state of a pin\n"
				<< "\t\t\trequires -p to be defined\n"
				<< "\n"
				<< "s=STATE, state=STATE\tsets the state of a pin\n"
				<< "\t\t\tStates:\n"
				<< "\t\t\t0, DISABLED\n"
				<< "\t\t\t1, ENABLED\n"
				<< "\t\t\trequires DIO and PIN to be defined\n"
				<< "\n"
				<< "m=MODE, mode=MODE\tsets the output mode of a specific dio port\n"
				<< "\t\t\tModes:\n"
				<< "\t\t\t0, PNP\n"
				<< "\t\t\t1, NPN\n"
				<< "\n"
				<< "Options:\n"
				<< "-p NUM, --pin NUM \tthe pin number to be used by COMMAND\n"
				<< "\n"
				<< "-d NUM, --dio NUM \tthe dio number to be used by COMMAND\n"
				<< "\t\t\tdefaults to 1 if not supplied\n"
				<< "\n"
				<< "-h, --human-readable \toutput data in a human readable format\n"
				<< "\n"
				<< "--help \t\t\tdisplay this help text and exit\n";
}

int main(int argc, char *argv[])
{
	// Create a list of args without optional switches
	// Allows for switches to be position independent
	bool human = false;
	int dio = 1, pin = -1;
	std::vector<std::string> argList;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--help")
		{
			showUsage();
			return 0;
		}
		else if (arg == "-p" || arg == "--pin")
		{
			if (i < argc - 1)
			{
				try { pin = std::stoi(std::string(argv[++i])); }
				catch (...) 
				{
					std::cerr << "Pin must be a number!" << std::endl;
					showUsage();
					return 1;
				}
			}
			else
			{
				std::cerr << "Missing pin number!" << std::endl;
				showUsage();
				return 1;
			}
		}
		else if (arg == "-d" || arg == "--dio")
		{
			if (i < argc - 1)
			{
				try { dio = std::stoi(std::string(argv[++i])); }
				catch (...)
				{
					std::cerr << "Dio must be a number!" << std::endl;
					showUsage();
					return 1;
				}
			}
			else
			{
				std::cerr << "Missing dio number!" << std::endl;
				showUsage();
				return 1;
			}
		}
		else if (arg == "-h" || arg == "--human-readable")
			human = true;
		else
			argList.emplace_back(arg);
	}

	if (argList.size() < 2)
	{
		showUsage();
		return 1;
	}

	if (!initDio(argList[0].data()))
	{
		printLastError();
		return 1;
	}

	std::string val;
	std::string& cmd = argList[1];	
	// If the command contains "=" we need to break it up into command / value pair.
	size_t index = cmd.find("=");
	if (index != cmd.npos)
	{
		val = cmd.substr(index + 1);
		cmd = cmd.substr(0, cmd.size() - val.size());
	}

	if (cmd == "s=" || cmd == "state=")
	{
		int state = stringToState(val);
		// stringToState returns -1 on error;
		if (state == -1)
		{
			std::cerr << "Invalid state supplied!!" << std::endl;
			showUsage();
			return 1;
		}

		if (digitalWrite(dio, pin, state) < 0)
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "s" || cmd == "state")
	{
		if (pin < 0)
		{
			std::cerr << "Pin argument must be supplied!!" << std::endl;
			showUsage();
			return 1;
		}

		int state = digitalRead(dio, pin);
		if (state >= 0)
		{
			if (human) printf("%s\n", stateToString(state));
			else printf("%d", state);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "m=" || cmd == "mode=")
	{
		OutputMode mode = stringToMode(val);
		//stringToMode returns 0 on error.
		if (mode == ModeError)
		{
			std::cerr << "Invalid mode supplied!!" << std::endl;
			showUsage();
			return 1;
		}

		if (setOutputMode(dio, mode) < 0)
		{
			printLastError();
			return 1;
		}
	}

	return 0;
}