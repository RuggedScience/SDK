#ifndef MOCK_DIO_CONTROLLER_H
#define MOCK_DIO_CONTROLLER_H

#include <gmock/gmock.h>

#include "controllers/abstractdiocontroller.h"

class MockDioController : public AbstractDioController {
   public:
    MOCK_METHOD(void, initPin, (const DioPinConfig&), (override));
    MOCK_METHOD(void, setPinState, (const DioPinConfig&, bool), (override));
    MOCK_METHOD(bool, getPinState, (const DioPinConfig&), (override));
    MOCK_METHOD(
        void,
        setPinMode,
        (const DioPinConfig&, rs::PinDirection),
        (override)
    );
    MOCK_METHOD(
        rs::PinDirection,
        getPinMode,
        (const DioPinConfig&),
        (override)
    );
};

#endif