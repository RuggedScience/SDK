#include "../poe/poe.h"
#include <iostream>

//This function goes into a loop to allow interactive control over the PoE ports
void interactivePoe()
{
	char cmd;
	int port, state;
	while (1)
	{
		std::cout << "Please enter a command" << std::endl;
		std::cout << "r = Read port state" << std::endl;
		std::cout << "s = Set port state" << std::endl;
		std::cout << "e = Exit" << std::endl;
		std::cin >> cmd;
		std::cin.clear();
		if (cmd == 'e') break;

		std::cout << "Please enter a port number" << std::endl;
		std::cin >> port;
		std::cin.clear();

		if (cmd == 'r')
		{
			PoeState ps = getPortState(port);
			if (ps == StateError)
			{
				std::cout << "Error: " << getLastPoeError() << std::endl;
			}
			else
			{
				std::cout << "Port " << port << " is ";
				switch (ps)
				{
					case StateEnabled:
						std::cout << "enabled";
						break;
					case StateDisabled:
						std::cout << "disabled";
						break;
					case StateAuto:
						std::cout << "auto";
						break;
				}
				std::cout << std::endl;
			}
		}
		else if (cmd == 's')
		{
			std::cout << "Enter desired state" << std::endl;
			std::cout << "0 = DISABLED" << std::endl;
			std::cout << "1 = ENABLED" << std::endl;
			std::cout << "2 = AUTO" << std::endl;
			std::cin >> state;
			std::cin.clear();
            if (setPortState(port, (PoeState)state) < 0)
			{
				std::cout << "Error: " << getLastPoeError() << std::endl; 
			}
		}
		else
		{
			std::cout << "Invalid command!!!" << std::endl;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "Provide an xml file" << std::endl;
		return 1;
	}

	if (initPoe(argv[1]))
		interactivePoe();
	else
		std::cout << getLastPoeError() << std::endl;

	return 0;
}