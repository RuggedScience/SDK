#include "rsdioimpl.h"
#include "rserrors.h"
#include "diocontroller.h"

#include <iostream>

std::string modeToString(rs::OutputMode mode)
{
    switch (mode) {
        case rs::OutputMode::Sink:
            return std::string("Sink");
        case rs::OutputMode::Source:
            return std::string("Source");
        default:
            return std::string("Error");
    }
}

int main()
{
    TestDioController *controller = new TestDioController();

    DioPinConfig sourcePin(-1, 0, 0, false, false, false, true);
    DioPinConfig sinkPin(-2, 0, 0, false, false, false, true);
    DioPinConfig outputPin(1, 2, 0, false, false, false, true);
    DioPinConfig inputPin(2, 3, 0, false, false, true, false);
    DioPinConfig dualPin(3, 4, 0, false, false, true, true);

    DioConnectorConfig connectorConfig = {
        .sinkPin = sinkPin,
        .sourcePin = sourcePin,
        .pins = {
            outputPin,
            inputPin,
            dualPin,
        }
    };

    DioControllerConfig controllerConfig = {
        .connectors = {
            connectorConfig
        }
    };

    RsDioImpl dio(controller, controllerConfig);

    
    try {
        dio.setOutputMode(0, rs::OutputMode::Sink);

    } catch (std::exception e) {
        e.what();
    }


    dio.setOutputMode(1, rs::OutputMode::Sink);

    rs::OutputMode mode = dio.getOutputMode(1);

    if (mode != rs::OutputMode::Sink) {
        std::cerr << "Expected OutputMode::Sink but got OutputMode::"
                  << modeToString(mode) << ".\n";
        return 1;
    }

    dio.setOutputMode(1, rs::OutputMode::Source);

    mode = dio.getOutputMode(1);
    if (mode != rs::OutputMode::Source) {
        std::cerr << "Expected OutputMode::Source but got OutputMode::"
                  << modeToString(mode) << ".\n";
        return 1;
    }

    dio.getOutputMode(2);

    dio.setOutputMode(2, rs::OutputMode::Sink);

    dio.digitalRead(0, 0);
    
    dio.digitalRead(1, 0);
    

    dio.digitalRead(1, 1);


    dio.digitalWrite(0, 0, false);
    

    dio.digitalWrite(1, 0, false);


    dio.digitalWrite(1, 2, false);
    

    dio.digitalWrite(1, 1, true);

    if (dio.digitalRead(1, 1) != true) {
        std::cerr
            << "digitalRead returned false after call to digitalWrite(true)"
            << std::endl;
        return 1;
    }

    dio.setPinDirection(1, 1, rs::PinDirection::Input);


    dio.setPinDirection(1, 1, rs::PinDirection::Output);


    dio.setPinDirection(1, 2, rs::PinDirection::Output);


    dio.setPinDirection(1, 2, rs::PinDirection::Input);

    dio.setPinDirection(1, 3, rs::PinDirection::Output);
    if (dio.getPinDirection(1, 3) != rs::PinDirection::Output) {
        std::cerr << "getPinDirection returned Input after call to "
                     "setPinDirection(Output)"
                  << std::endl;
        return 1;
    }

    dio.setPinDirection(1, 3, rs::PinDirection::Input);
    if (dio.getPinDirection(1, 3) != rs::PinDirection::Input) {
        std::cerr << "getPinDirection returned Output after call to "
                     "setPinDirection(Input)"
                  << std::endl;
        return 1;
    }

    std::map<int, bool> states = dio.readAll(1);

    if (states[1] != true) {
        std::cerr << "readAll: Expected state of 1 for pin 1 but got "
                  << states[1] << std::endl;
        return 1;
    }

    if (states[2] != false) {
        std::cerr << "readAll: Expected state of 0 for pin 2 but got "
                  << states[2] << std::endl;
    }

    return 0;
}