// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(5);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete door;
        delete adapter;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

TEST_F(TimedDoorTest, initiallyDoorIsClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, unlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, lockClosesDoor) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, returnsCorrectTimeout) {
    EXPECT_EQ(door->getTimeOut(), 5);
}

TEST_F(TimedDoorTest, adapterTimeoutThrowsWhenDoorOpen) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, adapterTimeoutNoThrowWhenDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, afterLockTimeoutDoesNothing) {
    door->unlock();
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, timeoutWorksAfterReopen) {
    door->unlock();
    door->lock();
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST(TimerClientMockSimpleTest, mockCanVerifyCall) {
    MockTimerClient mock;
    EXPECT_CALL(mock, Timeout()).Times(1);
    mock.Timeout();
}

TEST_F(TimedDoorTest, multipleUnlockKeepsDoorOpen) {
    door->unlock();
    door->unlock();
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, multipleLockKeepsDoorClosed) {
    door->unlock();
    door->lock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, timeoutDoesNotThrowAfterLockAndUnlock) {
    door->unlock();
    door->lock();
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, multipleTimeoutCallsAfterClose) {
    door->unlock();
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
    EXPECT_NO_THROW(adapter->Timeout());
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, adapterHoldsReferenceToSameDoor) {
    DoorTimerAdapter adapter2(*door);
    door->unlock();
    EXPECT_THROW(adapter2.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, timeoutAfterCloseAndReopen) {
    door->unlock();
    door->lock();
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}
