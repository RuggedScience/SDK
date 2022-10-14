#include <rsdio.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>

static int stringToState(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	if (str == "LOW" || str == "FALSE" || str == "0")
		return 0;
	else if (str == "HIGH" || str == "TRUE" || str == "1")
		return 1;
	
	return -1;
}

static const char *stateToString(bool state)
{
	if (!state)
		return "low";
	else
		return "high";
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
				<< "\t\t\t0, LOW\n"
				<< "\t\t\t1, HIGH\n"
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
				<< "--help \t\t\tdisplay this help text and exit\n"
				<< "--version \t\tdisplay library version information\n"
				<< "--debug \t\tprint debug information\n";
}

int main(int argc, char *argv[])
{
	std::shared_ptr<RsDio> rsdio(createRsDio(), std::mem_fn(&RsDio::destroy));
	if (!rsdio)
	{
		std::cerr << "Failed to create instance of RsDio" << std::endl;
		return 1;
	}

	// Create a list of args without optional switches
	// Allows for switches to be position independent
	bool human = false;
	bool debug = false;
	int dio = 1, pin = -1;
	std::vector<std::string> argList;
	std::vector<std::string> ignoredArgs;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "--help")
		{
			showUsage();
			return 0;
		}
		else if (arg == "--version")
		{
			std::cout << rsdio->version() << std::endl;
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
		else if (arg == "--debug")
			debug = true;
		else if (arg.find("-") == 0)
			ignoredArgs.emplace_back(arg);
		else
			argList.emplace_back(arg);
	}

	if (argList.size() < 2)
	{
		showUsage();
		return 1;
	}

	if (size_t s = ignoredArgs.size())
	{
		std::cerr << "Ignoring unknown argument";
		if (s == 1) std::cerr << ": ";
		else if (s > 1) std::cerr << "s: ";
		for (size_t i = 0; i < ignoredArgs.size(); ++i)
		{
			const std::string& arg = ignoredArgs[i];
			std::cerr << arg;
			if (i < ignoredArgs.size() - 1)
				std::cerr << ", ";
		}
		std::cerr << std::endl;
	}

	if (!rsdio->setXmlFile(argList[0].data(), debug))
	{
		std::cerr << rsdio->getLastError() << std::endl;
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

		if (rsdio->digitalWrite(dio, pin, state) < 0)
		{
			std::cerr << rsdio->getLastError() << std::endl;
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

		int state = rsdio->digitalRead(dio, pin);
		if (state >= 0)
		{
			if (human) printf("%s\n", stateToString(state));
			else printf("%d", state);
		}
		else
		{
			std::cerr << rsdio->getLastError() << std::endl;
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

		if (rsdio->setOutputMode(dio, mode) < 0)
		{
			std::cerr << rsdio->getLastError() << std::endl;
			return 1;
		}
	}
	else
	{
		std::cerr << "Invalid command supplied!!" << std::endl;
		showUsage();
		return 1;
	}

	return 0;
}
