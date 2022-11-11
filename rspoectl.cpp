#include <rspoe.h>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>

static rs::PoeState stringToState(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	//str = std::toupper((str.begin(), str.end());
	if (str == "DISABLED")
		return rs::PoeState::Disabled;
	else if (str == "ENABLED")
		return rs::PoeState::Enabled;
	else if (str == "AUTO")
		return rs::PoeState::Auto;
	
	return rs::PoeState::Error;
}

static const char *stateToString(rs::PoeState state)
{
    switch (state)
    {
        case rs::PoeState::Disabled:
            return "disabled";
        case rs::PoeState::Enabled:
            return "enabled";
        case rs::PoeState::Auto:
            return "auto";
    }

	return "error";
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
				<< "-h, --human-readable \toutput data in a human readable format\n"
				<< "--version\t\tdisplay library version information\n";
}

int main(int argc, char *argv[])
{
	// Create a list of args without optional switches
	// Allows for switches to be position independent
	bool human = false;
	std::vector<std::string> argList;
	std::vector<std::string> ignoredArgs;
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg == "help")
		{
			showUsage();
			return 0;
		}
		else if (arg == "--version")
		{
			std::cout << rs::rsPoeVersion() << std::endl;
			return 0;
		}
		else if (arg == "-h" || arg == "--human-readable")
			human = true;
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

	std::shared_ptr<rs::RsPoe> rspoe(rs::createRsPoe(), std::mem_fn(&rs::RsPoe::destroy));
	if (!rspoe)
	{
		std::cerr << "Failed to create instance of RsPoe" << std::endl;
	}

	if (!rspoe->setXmlFile(argList[0].data()))
	{
		std::cerr << rspoe->getLastErrorString() << std::endl;
		return 1;
	}

	int port = -1;
	if (argList.size() > 2)
	{
		try { port = std::stoi(argList[2]); }
		catch (...) {}
	}

	// If the command is "state=" we need to break it up into command / value.
	rs::PoeState state = rs::PoeState::Error;
	std::string& cmd = argList[1];	
	size_t index = cmd.find("=");
	if (index != cmd.npos)
	{
		std::string val = cmd.substr(index + 1);
		cmd = cmd.substr(0, cmd.size() - val.size());
		state = stringToState(val);

		// stringToState returs "StateError" if it couldn't convert it.
		// Let's check if they used a number instead of string.
		if (state == rs::PoeState::Error)
		{
			try
			{
				int s = std::stoi(val);
				if (s >= 0 && s < (int)rs::PoeState::Error)
					state = (rs::PoeState)s;
			}
			catch (...) {}
		}

		// Couldn't convert the state from a string or an int... give up.
		if (state == rs::PoeState::Error)
		{
			std::cerr << "Invalid state supplied" << std::endl;
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
		if (rspoe->setPortState(port, state) < 0)
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "s" || cmd == "state")
	{
		state = rspoe->getPortState(port);
		if (state != rs::PoeState::Error)
		{
			if (human) printf("%s\n", stateToString(state));
			else printf("%d", (int)state);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "v" || cmd == "voltage")
	{
		float voltage = rspoe->getPortVoltage(port);
		if (voltage >= 0.0f)
		{
			if (human) printf("%fV\n", voltage);
			else printf("%f", voltage);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "c" || cmd == "current")
	{
		float current = rspoe->getPortCurrent(port);
		if (current >= 0.0f)
		{
			if (human) printf("%fA\n", current);
			else printf("%f", current);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "w" || cmd == "wattage")
	{
		float watts = rspoe->getPortPower(port);
		if (watts >= 0.0f)
		{
			if (human) printf("%fW\n", watts);
			else printf("%f", watts);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "b" || cmd == "budget-consumed")
	{
		int consumed = rspoe->getBudgetConsumed();
		if (consumed >= 0)
		{
			if (human) printf("%dW\n", consumed);
			else printf("%d", consumed);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "a" || cmd == "budget-available")
	{
		int available = rspoe->getBudgetAvailable();
		if (available >= 0)
		{
			if (human) printf("%dW\n", available);
			else printf("%d", available);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
			return 1;
		}
	}
	else if (cmd == "t" || cmd == "budget-total")
	{
		int total = rspoe->getBudgetTotal();
		if (total >= 0)
		{
			if (human) printf("%dW\n", total);
			else printf("%d", total);
		}
		else
		{
			std::cerr << rspoe->getLastErrorString() << std::endl;
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
