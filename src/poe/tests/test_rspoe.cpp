#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mockpoecontroller.h"
#include "rserrors.h"
#include "rspoeimpl.h"

using namespace testing;

class RsPoeImplTest : public ::testing::Test {
   protected:
    PoeControllerConfig config;
    MockPoeController* mockController;
    std::unique_ptr<RsPoeImpl> impl;

    void SetUp() override
    {
        // Create strict mock to catch unexpected calls
        mockController = new StrictMock<MockPoeController>();
        config = createTestConfig();
        impl.reset(new RsPoeImpl(
            std::unique_ptr<AbstractPoeController>(mockController), config
        ));
    }

    PoeControllerConfig createTestConfig()
    {
        PoeControllerConfig c;
        c.controllerId = "mock";

        // Define Port 1 (ID=1) at hardware offset 0
        PoePortConfig p1;
        p1.id = 1;
        p1.offset = 0;
        c.ports.push_back(p1);

        // Define Port 2 (ID=2) at hardware offset 4 (e.g. 4-port controller)
        PoePortConfig p2;
        p2.id = 2;
        p2.offset = 4;
        c.ports.push_back(p2);

        return c;
    }
};

// --- TEST CASES ---

TEST_F(RsPoeImplTest, NotInitialized_Throws_NotInitialized)
{
    RsPoeImpl emptyImpl;

    EXPECT_THAT(
        []() { RsPoeImpl().getPortState(1); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::NotInitialized)
        )
    );
}

TEST_F(RsPoeImplTest, InvalidPort_Throws_InvalidParameter)
{
    EXPECT_THAT(
        [this]() { impl->getPortState(99); },
        Throws<rs::RsException>(
            Property(&rs::RsException::code, rs::RsErrorCode::InvalidParameter)
        )
    );
}

TEST_F(RsPoeImplTest, GetPortState_DelegatesToController)
{
    // Expect call with offset 0 (for Port ID 1)
    EXPECT_CALL(*mockController, getPortState(0))
        .WillOnce(Return(rs::PoeState::Enabled));

    rs::PoeState state = impl->getPortState(1);
    EXPECT_EQ(state, rs::PoeState::Enabled);
}

TEST_F(RsPoeImplTest, SetPortState_DelegatesToController)
{
    // Expect call with offset 4 (for Port ID 2)
    EXPECT_CALL(*mockController, setPortState(4, rs::PoeState::Disabled));

    impl->setPortState(2, rs::PoeState::Disabled);
}

TEST_F(RsPoeImplTest, GetPortMeasurements_DelegatesCorrectly)
{
    // Check mapping logic for ID 1 -> Offset 0
    EXPECT_CALL(*mockController, getPortVoltage(0)).WillOnce(Return(48.5f));
    EXPECT_CALL(*mockController, getPortCurrent(0)).WillOnce(Return(0.2f));
    EXPECT_CALL(*mockController, getPortPower(0)).WillOnce(Return(9.7f));

    EXPECT_FLOAT_EQ(impl->getPortVoltage(1), 48.5f);
    EXPECT_FLOAT_EQ(impl->getPortCurrent(1), 0.2f);
    EXPECT_FLOAT_EQ(impl->getPortPower(1), 9.7f);
}

TEST_F(RsPoeImplTest, ControllerError_IsRethrownAsRsException)
{
    // Simulate I2C failure in the controller
    EXPECT_CALL(*mockController, getBudgetConsumed())
        .WillOnce(
            Throw(std::system_error(std::make_error_code(std::errc::io_error)))
        );

    EXPECT_THAT(
        [this]() { impl->getBudgetConsumed(); },
        Throws<rs::RsException>(Property(&rs::RsException::code, rs::RsErrorCode::HardwareError))
    );
}

TEST_F(RsPoeImplTest, GetPortList_ReturnsConfiguredIDs)
{
    std::vector<int> ports = impl->getPortList();

    // We added ID 1 and ID 2 in createTestConfig
    ASSERT_EQ(ports.size(), 2);
    EXPECT_EQ(ports[0], 1);
    EXPECT_EQ(ports[1], 2);
}