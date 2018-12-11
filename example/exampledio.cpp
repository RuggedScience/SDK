#include "../dio/dio.h"
#include <iostream>

//This function goes into a loop to allow interactive control over the DIO
void interactiveDio(Dio &d)
{
	char cmd;
	int pin, state;
	while (1)
	{
		std::cout << "Please enter a command" << std::endl;
		std::cout << "r = Read pin" << std::endl;
		std::cout << "s = Set pin" << std::endl;
		std::cout << "e = Exit" << std::endl;
		std::cin >> cmd;
		std::cin.clear();
		if (cmd == 'e') break;

		std::cout << "Please enter a pin number" << std::endl;
		std::cin >> pin;
		std::cin.clear();

		try
		{
			if (cmd == 'r')
			{
				std::cout << "Pin " << pin << " is ";
				if (d.digitalRead(1, pin)) std::cout << "high";
				else std::cout << "low";
				std::cout << std::endl;
			}
			else if (cmd == 's')
			{
				std::cout << "Enter desired state" << std::endl;
				std::cout << "0 = LOW" << std::endl;
				std::cout << "1 = HIGH" << std::endl;
				std::cin >> state;
				std::cin.clear();
                d.digitalWrite(1, pin, state);
			}
			else
			{
				std::cout << "Invalid command!!!" << std::endl;
			}
		}
		catch (std::exception &ex)
		{
			std::cout << "ERROR: " << ex.what() << std::endl;
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
		std::cout << d.getLastErrorString() << std::endl;

	return 0;
}