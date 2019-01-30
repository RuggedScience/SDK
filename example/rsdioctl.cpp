#include "../dio/rsdio.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

static int stringToState(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	if (str == "DISABLED" || str == "FALSE")
		return 0;
	else if (str == "ENABLED" || str == "TRUE")
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

void printLastError()
{
	printf("Error: %s\n", getLastDioError());
}

static void showUsage()
{
	std::cout 	<< "Usage: rsdio FILE COMMAND DIO PIN [OPTIONS...]\n"
				<< "Commands:\n"
				<< "s, state\t\toutput the state of a pin\n"
				<< "\t\t\trequires DIO and PIN to be defined\n"
				<< "s, state=STATE\t\tsets the state of a pin\n"
				<< "\t\t\tStates:\n"
				<< "\t\t\t0, DISABLED\n"
				<< "\t\t\t1, ENABLED\n"
				<< "\t\t\trequires DIO and PIN to be defined\n"
				<< "help\t\t\tdisplay this help and exit\n"
				<< "Options:\n"
				<< "-h, --human-readable \toutput data in a human readable format\n";
}

int main(int argc, char *argv[])
{
	// Create a list of args without optional switches
	// Allows for switches to be position independent
	bool human = false;
	std::vector<std::string> argList;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "help")
		{
			showUsage();
			return 0;
		}
		else if (arg == "-h" || arg == "--human-readable")
			human = true;
		else
			argList.emplace_back(arg);
	}

	if (argList.size() < 4)
	{
		showUsage();
		return 1;
	}

	if (!initDio(argList[0].data()))
	{
		printLastError();
		return 1;
	}

	int dio, pin;
	try { dio = std::stoi(argList[2]); }
	catch (...)
	{
		std::cerr << "DIO must be a number!!" << std::endl;
		showUsage();
		return 1;
	}

	try { pin = std::stoi(argList[3]); }
	catch (...)
	{
		std::cerr << "PIN must be a number!!" << std::endl;
		showUsage();
		return 1;
	}

	// If the command is "state=" we need to break it up into command / value.
	int state;
	std::string& cmd = argList[1];	
	size_t index = cmd.find("=");
	if (index != cmd.npos)
	{
		std::string val = cmd.substr(index + 1);
		cmd = cmd.substr(0, cmd.size() - val.size());
		state = stringToState(val);

		// stringToState returs -1 if it couldn't convert it.
		// Let's check if they used a number instead of string.
		if (state == -1)
		{
			try { state = std::stoi(val); }
			catch (...) {}
		}

		// Couldn't convert the state from a string or an int... give up.
		if (state == -1)
		{
			std::cerr << "Invalid state supplied!!" << std::endl;
			showUsage();
			return 1;
		}
	}

	if (cmd == "s=" || cmd == "state=")
	{
		if (digitalWrite(dio, pin, state) < 0)
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "s" || cmd == "state")
	{
		state = digitalRead(dio, pin);
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

	return 0;
}