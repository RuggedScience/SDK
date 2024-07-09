#include <iostream>

#include "../dio/src/rsdioimpl.h"
#include "diocontroller.h"
#include "utils.h"

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

    PinConfig sourcePin(0, 0, false, false, false, true);
    PinConfig sinkPin(1, 0, false, false, false, true);
    PinConfig outputPin(2, 0, false, false, false, true);
    PinConfig inputPin(3, 0, false, false, true, false);
    PinConfig dualPin(4, 0, false, false, true, true);

    pinconfigmap_t pinMap = {
        {-1, sourcePin},
        {-2, sinkPin},
        {1, outputPin},
        {2, inputPin},
        {3, dualPin}
    };

    dioconfigmap_t dioMap = {{1, pinMap}, {2, {}}};
    RsDioImpl dio(controller, dioMap);

    dio.setOutputMode(0, rs::OutputMode::Sink);
    verifyError(
        "setOutputMode (invalid dio)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.setOutputMode(1, rs::OutputMode::Sink);
    verifyError("setOutputMode (Sink)", dio.getLastError());

    rs::OutputMode mode = dio.getOutputMode(1);
    verifyError("getOutputMode (Sink)", dio.getLastError());

    if (mode != rs::OutputMode::Sink) {
        std::cerr << "Expected OutputMode::Sink but got OutputMode::"
                  << modeToString(mode) << ".\n";
        return 1;
    }

    dio.setOutputMode(1, rs::OutputMode::Source);
    verifyError("setOutputMode (Source)", dio.getLastError());

    mode = dio.getOutputMode(1);
    if (mode != rs::OutputMode::Source) {
        std::cerr << "Expected OutputMode::Source but got OutputMode::"
                  << modeToString(mode) << ".\n";
        return 1;
    }

    dio.getOutputMode(2);
    verifyError(
        "getOutputMode (unsupported dio)",
        dio.getLastError(),
        std::errc::function_not_supported
    );

    dio.setOutputMode(2, rs::OutputMode::Sink);
    verifyError(
        "setOutputMode (unsupported dio)",
        dio.getLastError(),
        std::errc::function_not_supported
    );

    dio.digitalRead(0, 0);
    verifyError(
        "digitalRead (invalid dio)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.digitalRead(1, 0);
    verifyError(
        "digitalRead (invalid pin)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.digitalRead(1, 1);
    verifyError("digitalRead (valid)", dio.getLastError());

    dio.digitalWrite(0, 0, false);
    verifyError(
        "digitalWrite (invalid dio)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.digitalWrite(1, 0, false);
    verifyError(
        "digitalWrite (invalid pin)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.digitalWrite(1, 2, false);
    verifyError(
        "digitalWrite (unsupported pin)",
        dio.getLastError(),
        std::errc::function_not_supported
    );

    dio.digitalWrite(1, 1, true);
    verifyError("digitalWrite (valid)", dio.getLastError());

    if (dio.digitalRead(1, 1) != true) {
        std::cerr
            << "digitalRead returned false after call to digitalWrite(true)"
            << std::endl;
        return 1;
    }

    dio.setPinDirection(1, 1, rs::PinDirection::Input);
    verifyError(
        "setPinDirection (output pin)",
        dio.getLastError(),
        std::errc::function_not_supported
    );

    dio.setPinDirection(1, 1, rs::PinDirection::Output);
    verifyError("setPinDirection (output pin)", dio.getLastError());

    dio.setPinDirection(1, 2, rs::PinDirection::Output);
    verifyError(
        "setPinDirection (input pin)",
        dio.getLastError(),
        std::errc::function_not_supported
    );

    dio.setPinDirection(1, 2, rs::PinDirection::Input);
    verifyError("setPinDirection (input pin)", dio.getLastError());

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
    verifyError("readAll (valid)", dio.getLastError());

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