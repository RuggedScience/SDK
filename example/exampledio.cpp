#include "../dio/dio.h"
#include <iostream>

//This function goes into a loop to allow interactive control over the DIO
void interactiveDio()
{
	if (setOutputMode(1, ModeNpn) < 0)
		std::cout << getLastDioError() << std::endl;

	if (setOutputMode(2, ModePnp) < 0)
		std::cout << getLastDioError() << std::endl;

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
			state = digitalRead(dio, pin);
			if (state < 0)
			{
				std::cout << "Error: " << getLastDioError() << std::endl;
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

			if (digitalWrite(dio, pin, state) < 0)
				std::cout << "Error: " << getLastDioError() << std::endl;
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

	if (initDio(argv[1]))
		interactiveDio();
	else
		std::cout << getLastDioError() << std::endl;

	return 0;
}