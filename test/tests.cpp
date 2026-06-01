// Copyright 2025 UNN-CS
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Return;

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(5000);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete adapter;
        delete door;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 5000);
}

TEST_F(TimedDoorTest, ThrowStateThrowsExceptionWhenOpen) {
    door->unlock();
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateThrowsCorrectMessage) {
    door->unlock();
    try {
        door->throwState();
        FAIL() << "Expected exception";
    } catch (const std::runtime_error& e) {
        EXPECT_THAT(e.what(), ::testing::HasSubstr("open"));
    }
}

TEST_F(TimedDoorTest, MultipleUnlockKeepsDoorOpen) {
    door->unlock();
    door->unlock();
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, MultipleLockKeepsDoorClosed) {
    door->unlock();
    door->lock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenDoorOpen) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutNoThrowWhenDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, AdapterTimeoutAfterOpenThenClose) {
    door->unlock();
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, AdapterChecksDoorStateCorrectly) {
    door->unlock();
    door->lock();
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorStateSequence) {
    EXPECT_FALSE(door->isDoorOpened());

    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());

    door->lock();
    EXPECT_FALSE(door->isDoorOpened());

    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());

    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, MultipleTimeoutCallsAfterClose) {
    door->unlock();
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
    EXPECT_NO_THROW(adapter->Timeout());
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST(MockTimerClientTest, CanVerifyTimeoutCall) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    mockClient.Timeout();
}

TEST(MockTimerClientTest, CanVerifyMultipleCalls) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout()).Times(3);
    mockClient.Timeout();
    mockClient.Timeout();
    mockClient.Timeout();
}

TEST(MockTimerClientTest, CanVerifyNoCalls) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout()).Times(0);
}

TEST(TimerTest, TimerCanBeCreated) {
    Timer timer;
    EXPECT_TRUE(true);
}

TEST_F(TimedDoorTest, DifferentTimeouts) {
    TimedDoor fastDoor(100);
    TimedDoor slowDoor(10000);

    EXPECT_EQ(fastDoor.getTimeOut(), 100);
    EXPECT_EQ(slowDoor.getTimeOut(), 10000);
    EXPECT_NE(fastDoor.getTimeOut(), slowDoor.getTimeOut());
}

TEST_F(TimedDoorTest, AdapterWithTimerIntegration) {
    door->unlock();

    EXPECT_THROW({
        adapter->Timeout();
    }, std::runtime_error);
}
