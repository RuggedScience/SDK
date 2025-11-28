#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockdiocontroller.h"
#include "rsdioimpl.h"
#include "rserrors.h"

using namespace testing;

class RsDioImplTest : public ::testing::Test {
   protected:
    DioControllerConfig config;
    MockDioController* mockController;
    std::unique_ptr<RsDioImpl> impl;

    void SetUp() override
    {
        // Create strict mock to catch unexpected calls
        mockController = new StrictMock<MockDioController>();
        config = createTestConfig();

        // Since we are using a strict mock, we need to add this
        // so it doesn't throw an error saying the call was over-saturated.
        EXPECT_CALL(*mockController, initPin(_)).Times(4);
        impl.reset(new RsDioImpl(
            std::unique_ptr<AbstractDioController>(mockController), config
        ));
    }

    DioControllerConfig createTestConfig()
    {
        // Main config
        DioControllerConfig c;
        c.controllerId = "mock";
        // DIO connector 1 config (supports output mode config)
        DioConnectorConfig conn1 = {
            .connectorId = 1,
            .sinkPin =
                DioPinConfig(SINK_PIN_ID, 0, 0, false, false, false, true),
            .sourcePin =
                DioPinConfig(SOURCE_PIN_ID, 0, 0, false, false, false, true),
            .pins = {
                DioPinConfig(1, 0, 0, false, false, true, false),  // Input pin
                DioPinConfig(2, 0, 0, false, false, false, true),  // Output pin
                DioPinConfig(3, 0, 0, false, false, true, true)    // Bi-Di pin
            }
        };
        c.connectors.push_back(conn1);

        // DIO connector 2 config (does not support output mode config)
        DioConnectorConfig conn2 = {
            .connectorId = 2,
            .pins = {
                DioPinConfig(1, 0, 0, false, false, true, true),
            }
        };
        c.connectors.push_back(conn2);
        return c;
    }
};

// --- TEST CASES ---

TEST_F(RsDioImplTest, NotInitialized_Throws_NotInitializedError)
{
    EXPECT_THAT(
        [] { RsDioImpl().getPinList(); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::NotInitialized)
        )
    );
}

TEST_F(RsDioImplTest, CanSetOutputMode_ReturnsCorrectValue)
{
    ASSERT_TRUE(impl->canSetOutputMode(1));
    ASSERT_FALSE(impl->canSetOutputMode(2));
}

TEST_F(RsDioImplTest, CanSetOutputMode_ThrowsInvalidParameter)
{
    EXPECT_THAT(
        [this] { impl->canSetOutputMode(3); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::InvalidParameter)
        )
    );
}

TEST_F(RsDioImplTest, SetOutputMode_Sink_CallsController)
{
    EXPECT_CALL(
        *mockController,
        setPinState(Field(&DioPinConfig::id, SINK_PIN_ID), true)
    );
    EXPECT_CALL(
        *mockController,
        setPinState(Field(&DioPinConfig::id, SOURCE_PIN_ID), false)
    );

    impl->setOutputMode(1, rs::OutputMode::Sink);
}

TEST_F(RsDioImplTest, SetOutputMode_Source_CallsController)
{
    EXPECT_CALL(
        *mockController,
        setPinState(Field(&DioPinConfig::id, SINK_PIN_ID), false)
    );
    EXPECT_CALL(
        *mockController,
        setPinState(Field(&DioPinConfig::id, SOURCE_PIN_ID), true)
    );

    impl->setOutputMode(1, rs::OutputMode::Source);
}

TEST_F(RsDioImplTest, SetOutputMode_ThrowsInvalidParameter)
{
    EXPECT_THAT(
        [this] { impl->setOutputMode(3, rs::OutputMode::Sink); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::InvalidParameter)
        )
    );
}

TEST_F(RsDioImplTest, SetOutputMode_ThrowsUnsupported)
{
    EXPECT_THAT(
        [this] { impl->setOutputMode(2, rs::OutputMode::Sink); },
        Throws<rs::RsException>(Property(
            &rs::RsException::code, rs::RsErrorCode::UnsupportedFunction
        ))
    );
}

TEST_F(RsDioImplTest, DigitalWrite_OutputPin_CallsController)
{
    EXPECT_CALL(
        *mockController, setPinState(Field(&DioPinConfig::id, 2), true)
    );
    impl->digitalWrite(1, 2, true);
}

TEST_F(RsDioImplTest, DigitalWrite_InputPin_ThrowsUnsupported)
{
    // We expect NO calls to the hardware because validation should fail first
    EXPECT_CALL(*mockController, setPinState(_, _)).Times(0);

    EXPECT_THAT(
        [this] { impl->digitalWrite(1, 1, true); },
        Throws<rs::RsException>(Property(
            &rs::RsException::code, rs::RsErrorCode::UnsupportedFunction
        ))
    );
}

TEST_F(RsDioImplTest, DigitalWrite_ThrowsInvalidParameter)
{
    EXPECT_THAT(
        [this] { impl->digitalWrite(1, 99, true); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::InvalidParameter)
        )
    );
}

TEST_F(RsDioImplTest, DigitalRead_ReturnsValueFromController)
{
    // Setup Expectation: "When getPinState is called for Pin 1, return true"
    EXPECT_CALL(*mockController, getPinState(Field(&DioPinConfig::id, 1)))
        .WillOnce(Return(true));

    bool val = impl->digitalRead(1, 1);
    EXPECT_TRUE(val);
}

TEST_F(RsDioImplTest, HardwareError_BubblesUp)
{
    // Simulate a hardware failure (exception thrown by controller)
    EXPECT_CALL(*mockController, getPinState(_))
        .WillOnce(
            Throw(std::system_error(std::make_error_code(std::errc::io_error)))
        );

    EXPECT_THAT(
        [this] { impl->digitalRead(1, 1); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::HardwareError)
        )
    );
}