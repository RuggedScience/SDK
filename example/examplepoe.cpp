#include "../poe/rspoe.h"
#include <iostream>
#include <cstdio>

void printLastError()
{
	printf("Error: %s\nw", getLastPoeError());
}

//This function goes into a loop to allow interactive control over the PoE ports
void interactivePoe()
{

	printf("\nBudget Consumed: %d\n", getBudgetConsumed());
	printf("Budget Available: %d\n", getBudgetAvailable());
	printf("Budget Total: %d\n\n", getBudgetTotal());

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
			{
				printf("Port %d is ", port);
				switch (ps)
				{
					case StateEnabled:
						printf("enabled\n");
						break;
					case StateDisabled:
						printf("disabled\n");
						break;
					case StateAuto:
						printf("auto\n");
						break;
				}
			}
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

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Provide an xml file\n");
		return 1;
	}

	if (initPoe(argv[1]))
		interactivePoe();
	else
		printLastError();

	return 0;
}