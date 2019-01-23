#include "../dio/rsdio.h"
#include <iostream>
#include <cstdio>

void printLastError()
{
	printf("Error: %s\n", getLastDioError());
}

//This function goes into a loop to allow interactive control over the DIO
void interactiveDio()
{
	if (setOutputMode(1, ModeNpn) < 0) 
		printLastError();
	if (setOutputMode(2, ModePnp) < 0) 
		printLastError();

	char cmd;
	int dio, pin, state;
	while (1)
	{
		printf("Please enter a command\n");
		printf("r = Read pin\n");
		printf("s = Set pin\n");
		printf("e = Exit\n");
		std::cin >> cmd;
		std::cin.clear();
		if (cmd == 'e') break;

		printf("Please enter a dio number\n");
		std::cin >> dio;
		std::cin.clear();

		printf("Please enter a pin number\n");
		std::cin >> pin;
		std::cin.clear();

		if (cmd == 'r')
		{
			state = digitalRead(dio, pin);
			if (state >= 0)
				printf("Pin %d is %s\n", pin, state ? "high" : "low");
			else 
				printLastError();
		}
		else if (cmd == 's')
		{
			printf("Enter desired state\n");
			printf("0 = LOW\n");
			printf("1 = HIGH\n");
			std::cin >> state;
			std::cin.clear();

			if (digitalWrite(dio, pin, state) < 0) 
				printLastError();
		}
		else 
			printf("Invalid command!!!\n");
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Provide an xml file\n");
		return 1;
	}

	if (initDio(argv[1]))
		interactiveDio();
	else
		std::cout << getLastDioError() << std::endl;

	return 0;
}