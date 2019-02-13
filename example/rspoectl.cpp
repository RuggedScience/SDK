#include "../poe/rspoe.h"
#include <iostream>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>

static PoeState stringToState(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	//str = std::toupper((str.begin(), str.end());
	if (str == "DISABLED")
		return StateDisabled;
	else if (str == "ENABLED")
		return StateEnabled;
	else if (str == "AUTO")
		return StateAuto;
	
	return StateError;
}

static const char *stateToString(PoeState state)
{
    switch (state)
    {
        case StateDisabled:
            return "disabled";
        case StateEnabled:
            return "enabled";
        case StateAuto:
            return "auto";
    }

	return "error";
}

static void printLastError()
{
	std::cerr << getLastPoeError() << std::endl;
}

static void showUsage()
{
	std::cout 	<< "Usage: rspoectl FILE COMMAND [PORT] [OPTIONS...]\n"
				<< "Commands:\n"
				<< "s, state\t\toutput the state of a port\n"
				<< "\t\t\trequires PORT to be defined\n"
				<< "s=STATE, state=STATE\t\tsets the state of a port\n"
				<< "\t\t\tStates:\n"
				<< "\t\t\t0, DISABLED\n"
				<< "\t\t\t1, ENABLED\n"
				<< "\t\t\t2, AUTO\n"
				<< "\t\t\trequires PORT to be defined\n"
				<< "v, voltage\t\toutput the voltage in volts of a port\n"
				<< "\t\t\trequires PORT to be defined\n"
				<< "c, current\t\toutput the current in amps of a port\n"
				<< "\t\t\trequires PORT to be defined\n"
				<< "w, wattage\t\toutput the wattage in watts of a port\n"
				<< "\t\t\trequires PORT to be defined\n"
				<< "b, budget-consumed\toutput the consumed budget in watts\n"
				<< "a, budget-available\toutput the available budget in watts\n"
				<< "t, budget-total\t\toutput the total budget in watts\n"
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

	if (argList.size() < 2)
	{
		showUsage();
		return 1;
	}

	if (!initPoe(argList[0].data()))
	{
		printLastError();
		return 1;
	}

	int port = -1;
	if (argList.size() > 2)
	{
		try { port = std::stoi(argList[2]); }
		catch (...) {}
	}

	// If the command is "state=" we need to break it up into command / value.
	PoeState state = StateError;
	std::string& cmd = argList[1];	
	size_t index = cmd.find("=");
	if (index != cmd.npos)
	{
		std::string val = cmd.substr(index + 1);
		cmd = cmd.substr(0, cmd.size() - val.size());
		state = stringToState(val);

		// stringToState returs "StateError" if it couldn't convert it.
		// Let's check if they used a number instead of string.
		if (state == StateError)
		{
			try
			{
				int s = std::stoi(val);
				if (s >= 0 && s < (int)StateError)
					state = (PoeState)s;
			}
			catch (...) {}
		}

		// Couldn't convert the state from a string or an int... give up.
		if (state == StateError)
		{
			std::cerr << "Invalid state supplied!!" << std::endl;
			showUsage();
			return 1;
		}
	}

	// The below commands REQUIRE the PORT option.
	// So we need to quit if no PORT option was given with those commands.
	if (port == -1 && (
		cmd == "s=" || cmd == "state=" 	||
		cmd == "s" 	|| cmd == "state" 	||
		cmd == "v" 	|| cmd == "voltage"	||
		cmd == "c" 	|| cmd == "current" ||
		cmd == "w" 	|| cmd == "wattage"))
	{
		std::cerr << "Port required for '" << cmd << "' command!!" << std::endl;
		showUsage();
		return 1;
	}

	if (cmd == "s=" || cmd == "state=")
	{
		if (setPortState(port, state) < 0)
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "s" || cmd == "state")
	{
		state = getPortState(port);
		if (state != StateError)
		{
			if (human) printf("%s\n", stateToString(state));
			else printf("%d", (int)state);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "v" || cmd == "voltage")
	{
		float voltage = getPortVoltage(port);
		if (voltage >= 0.0f)
		{
			if (human) printf("%fV\n", voltage);
			else printf("%f", voltage);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "c" || cmd == "current")
	{
		float current = getPortCurrent(port);
		if (current >= 0.0f)
		{
			if (human) printf("%fA\n", current);
			else printf("%f", current);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "w" || cmd == "wattage")
	{
		float watts = getPortPower(port);
		if (watts >= 0.0f)
		{
			if (human) printf("%fW\n", watts);
			else printf("%f", watts);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "b" || cmd == "budget-consumed")
	{
		int consumed = getBudgetConsumed();
		if (consumed >= 0)
		{
			if (human) printf("%dW\n", consumed);
			else printf("%d", consumed);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "a" || cmd == "budget-available")
	{
		int available = getBudgetAvailable();
		if (available >= 0)
		{
			if (human) printf("%dW\n", available);
			else printf("%d", available);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "t" || cmd == "budget-total")
	{
		int total = getBudgetTotal();
		if (total >= 0)
		{
			if (human) printf("%dW\n", total);
			else printf("%d", total);
		}
		else
		{
			printLastError();
			return 1;
		}
	}
	else
	{
		showUsage();
		return 1;
	}

	return 0;
}
