#ifndef MOCK_POE_CONTROLLER_H
#define MOCK_POE_CONTROLLER_H

#include <gmock/gmock.h>

#include "controllers/abstractpoecontroller.h"

class MockPoeController : public AbstractPoeController {
   public:
    // MOCK_METHOD(ReturnType, MethodName, (Args...), (Specifiers));

    MOCK_METHOD(rs::PoeState, getPortState, (uint8_t), (override));
    MOCK_METHOD(void, setPortState, (uint8_t, rs::PoeState), (override));

    MOCK_METHOD(float, getPortVoltage, (uint8_t), (override));
    MOCK_METHOD(float, getPortCurrent, (uint8_t), (override));
    MOCK_METHOD(float, getPortPower, (uint8_t), (override));

    MOCK_METHOD(int, getBudgetConsumed, (), (override));
    MOCK_METHOD(int, getBudgetAvailable, (), (override));
    MOCK_METHOD(int, getBudgetTotal, (), (override));
};

#endif