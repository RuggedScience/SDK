#include "../dio/dio.h"
#include <iostream>

//This function goes into a loop to allow interactive control over the DIO
void interactiveDio(Dio &d)
{
	d.setOutputMode(1, Dio::ModeNpn);

	char cmd;
	int dio, pin, state;
	while (1)
	{
		std::cout << "Please enter a command" << std::endl;
		std::cout << "r = Read pin" << std::endl;
		std::cout << "s = Set pin" << std::endl;
		std::cout << "e = Exit" << std::endl;
		std::cin >> cmd;
		std::cin.clear();
		if (cmd == 'e') break;

		std::cout << "Please enter a dio number" << std::endl;
		std::cin >> dio;
		std::cin.clear();

		std::cout << "Please enter a pin number" << std::endl;
		std::cin >> pin;
		std::cin.clear();

		if (cmd == 'r')
		{
			state = d.digitalRead(dio, pin);
			if (state < 0)
			{
				std::cout << "Error: " << d.getLastError() << std::endl;
			}
			else
			{
				std::cout << "Pin " << pin << " is ";
				if (state) std::cout << "high";
				else std::cout << "low";
				std::cout << std::endl;
			}
		}
		else if (cmd == 's')
		{
			std::cout << "Enter desired state" << std::endl;
			std::cout << "0 = LOW" << std::endl;
			std::cout << "1 = HIGH" << std::endl;
			std::cin >> state;
			std::cin.clear();

			if (d.digitalWrite(dio, pin, state))
				std::cout << "Error: " << d.getLastError() << std::endl;
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

	Dio d(argv[1]);
	if (d.open())
		interactiveDio(d);
	else
		std::cout << d.getLastError() << std::endl;

	return 0;
}