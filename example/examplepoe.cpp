#include "../poe/rspoe.h"
#include <iostream>
#include <cstdio>
#include <cctype>

static PoeState stringToState(std::string str)
{
	str = str.toupper(str.begin(), str.end());
	switch (str)
	{
		case "DISABLED":
			return StateDisabled;
		case "ENABLED":
			return StateEnabled;
		case "AUTO":
			return StateAuto;
		default:
			return StateError;
	}
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
        case StateError:
            return "error";
    }
}

static void printLastError()
{
	std::cerr << getLastPoeError() << std::endl;
}

//This function goes into a loop to allow interactive control over the PoE ports
static void interactivePoe()
{
	int val = getBudgetConsumed();
	if (val >= 0) printf("\nBudget Consumed: %d\n", val)
	else printLastError();

	val = getBudgetAvailable();
	if (val >= 0) printf("Budget Available: %d\n", val);
	else printLastError();

	val = getBudgetTotal();
	if (val >= 0) printf("Budget Total: %d\n\n", val);
	else printLastError();

	char cmd;
	int port, state;
	while (1)
	{
		printf("Please enter a command\n");
		printf("r = Read port state\n");
		printf("s = Set port state\n");
		printf("v = Read port voltage\n");
		printf("c = Read port current\n");
		printf("w = Read port wattage\n");
		printf("e = Exit\n");
		std::cin >> cmd;
		std::cin.clear();
		if (cmd == 'e') break;

		printf("Please enter a port number\n");
		std::cin >> port;
		std::cin.clear();

		if (cmd == 'r')
		{
			PoeState ps = getPortState(port);
			if (ps != StateError)
				printf("Port %d state: %s\n", port, stateToString(state));
			else 
				printLastError();
		}
		else if (cmd == 's')
		{
			printf("Enter desired state\n");
			printf("0 = DISABLED\n");
			printf("1 = ENABLED\n");
			printf("2 = AUTO\n");
			std::cin >> state;
			std::cin.clear();
            if (setPortState(port, (PoeState)state) < 0) 
				printLastError();
		}
		else if (cmd == 'v')
		{
			float v = getPortVoltage(port);

			if (v < 0) 
				printLastError();
			else 
				printf("Port %d voltage: %fv\n", port, v);
		}
		else if (cmd == 'c')
		{
			float c = getPortCurrent(port);

			if (c < 0)
				printLastError();
			else 
				printf("Port %d current: %fa\n", port, c);
		}
		else if (cmd == 'w')
		{
			float w = getPortPower(port);

			if (w < 0) 
				printLastError();
			else 
				printf("Port %d wattage: %fw\n", port, w);
		}
		else
			printf("Invalid Command!!!\n");
	}
}

static void showUsage()
{
	std::cout 	<< "Usage: " argv[0] << " FILE COMMAND [PORT] [OPTIONS]...\n"
				<< "Commands:\n"
				<< "-s, --state\t\toutput the state of a port\n"
				<< "\t\t\t\trequires PORT to be defined\n"
				<< "-s, --state=STATE\t\tsets the state of a port\n"
				<< "\t\t\t\tStates:\n"
				<< "\t\t\t\t0, DISABLED\n"
				<< "\t\t\t\t1, ENABLED\n"
				<< "\t\t\t\t2, AUTO\n"
				<< "\t\t\t\tif PORT is not supplied all ports will be set to STATE\n"
				<< "-v, --voltage\t\toutput the voltage in volts of a port\n"
				<< "\t\t\t\trequires PORT to be defined\n"
				<< "-c, --current\t\toutput the current in amps of a port\n"
				<< "\t\t\t\trequires PORT to be defined\n"
				<< "-w, --wattage\t\toutput the wattage in watts of a port\n"
				<< "\t\t\t\trequires PORT to be defined\n"
				<< "-b, --budget-consumed\t\toutput the consumed budget in watts\n"
				<< "-a, --budget-available\t\toutput the available budget in watts\n"
				<< "-t, --budget-total\t\toutput the total budget in watts\n"
				<< "-i, --interactive\t\tenter interactive control mode\n"
				<< "--help\t\tdisplay this help and exit\n";
				<< "Options:\n"
				<< "-h, --human-readable \t\toutput data in a human readable format\n"
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		showUsage();
		return 1;
	}

	if (!initPoe(argv[1]))
	{
		printLastError();
		return 1;
	}

	std::string cmd = argv[2];

	if (cmd == "-i" || cmd == "--interactive")
	{
		interactivePoe();
		return 0;
	}
	else if (cmd == "--help")
	{
		showUsage();
		return 0;
	}

	int port = -1;
	std::string arg = argv[3];
	try	{ port = std::stoi(arg); }
	catch (std::exception &ex) { }

	// If no port is defined we need to look for options in place of it.
	if (port == -1) i = 3;
	else i = 4;

	bool human = false;
	for (; i < argc, ++i)
	{
		arg = argv[i];
		if (arg == "-h" || arg == "--human-readable")
			human = true;
	}

	if (port == -1 && (
		cmd == "-s" || cmd == "--state" ||
		cmd == "-v" || cmd == "--voltage" ||
		cmd == "-c" || cmd == "--current" ||
		cmd == "-w" || cmd == "--wattage"
	))
	{
		showUsage();
		return 1;
	}

	PoeState state = StateError;
	size_t i = cmd.find("=");
	if (i != cmd.npos)
	{
		std::string val = cmd.substr(i + 1);
		cmd = cmd.substr(0, cmd.size() - val.size());
		state = stringToState(val);

		if (state == StateError)
		{
			try
			{
				int s = std::stoi(val);
				if (s > 0 && s < (int)StateError)
					state = (PoeState)s;
			}
			catch (std::exception& ex) {}
		}
	}

	if (state == StateError)
	{
		showUsage();
		return 1;
	}

	if (cmd == "-s=" || cmd == "--state=")
	{
		if (setPortState(port, state) < 0)
		{
			printLastError();
			return 1;
		}
	}
	else if (cmd == "-s" || cmd == "--state")
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
	else if (cmd == "-v" || cmd == "--voltage")
	{
		float voltage = getPortVoltage(port);
		if (voltage >= 0.0f)
		{
			if (human) printf("%fV\n", voltage);
			else printf("%f", voltage);
		}
		else
		{
			printLastError()
			return 1;
		}
	}
	else if (cmd == "-c" || cmd == "--current")
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
	else if (cmd == "-w" || cmd == "--wattage")
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
	else if (cmd == "-b" || cmd == "--budget-consumed")
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
	else if (cmd == "-a" || cmd == "--budget-available")
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
	else if (cmd == "-t" || cmd == "--budget-total")
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