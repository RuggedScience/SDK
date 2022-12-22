#include <iostream>

#include "../dio/src/rsdioimpl.h"
#include "diocontroller.h"
#include "utils.h"

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
        {3, dualPin}};

    dioconfigmap_t dioMap = {{1, pinMap}, {2, {}}};
    RsDioImpl dio(controller, dioMap);

    dio.setOutputMode(0, rs::OutputMode::Sink);
    verifyError(
        "setOutputMode (invalid dio)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.setOutputMode(1, rs::OutputMode::Error);
    verifyError(
        "setOutputMode (invalid mode)",
        dio.getLastError(),
        std::errc::invalid_argument
    );

    dio.setOutputMode(1, rs::OutputMode::Sink);
    verifyError("setOutputMode (valid)", dio.getLastError());

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
            << "digitalRead returned false after call to  digitalWrite(true)"
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