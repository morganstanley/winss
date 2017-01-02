#include <functional>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/supervise/state_listener.hpp"
#include "winss/supervise/supervise.hpp"
#include "winss/supervise/controller.hpp"
#include "mock_state_file.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::Invoke;

namespace winss {
class SuperviseStateListenerTest : public testing::Test {
};

TEST_F(SuperviseStateListenerTest, IsEnabled) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener1(state_file, WAIT_UP);
    EXPECT_TRUE(state_listener1.IsEnabled());
    winss::SuperviseStateListener state_listener2(state_file, NO_WAIT);
    EXPECT_FALSE(state_listener2.IsEnabled());
}

TEST_F(SuperviseStateListenerTest, WaitUp) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_UP);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = false;
        state->is_run_process = false;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_TRUE(state_listener.CanStart());

    EXPECT_TRUE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseEnd
    }));

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseRun
    }));

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseRun
    }));
}

TEST_F(SuperviseStateListenerTest, WaitUpAlreadyUp) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_UP);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = true;
        state->is_run_process = true;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_FALSE(state_listener.CanStart());
}

TEST_F(SuperviseStateListenerTest, WaitDown) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_DOWN);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = true;
        state->is_run_process = true;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_TRUE(state_listener.CanStart());

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseEnd
    }));
}

TEST_F(SuperviseStateListenerTest, WaitDownAlreadyDown) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_DOWN);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = true;
        state->is_run_process = false;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_FALSE(state_listener.CanStart());
}

TEST_F(SuperviseStateListenerTest, WaitFinished) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_FINISHED);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = true;
        state->is_run_process = false;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_TRUE(state_listener.CanStart());

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseFinished
    }));
}

TEST_F(SuperviseStateListenerTest, WaitFinishedAlreadyFinished) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_FINISHED);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = false;
        state->is_run_process = false;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_FALSE(state_listener.CanStart());
}

TEST_F(SuperviseStateListenerTest, WaitRestart) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_RESTART);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = true;
        state->is_run_process = true;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_TRUE(state_listener.CanStart());

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseEnd,
        winss::SuperviseController::kSuperviseFinished,
        winss::SuperviseController::kSuperviseRun
    }));
}

TEST_F(SuperviseStateListenerTest, WaitRestartWhenFinished) {
    NiceMock<winss::MockSuperviseStateFile> state_file("test");
    winss::SuperviseStateListener state_listener(state_file, WAIT_RESTART);

    EXPECT_CALL(state_file, Read(_))
        .WillOnce(DoAll(Invoke([](winss::SuperviseState* state) {
        state->is_up = false;
        state->is_run_process = false;
    }), Return(true)));

    state_listener.HandleConnected();

    EXPECT_TRUE(state_listener.CanStart());

    EXPECT_FALSE(state_listener.HandleRecieved({
        winss::SuperviseController::kSuperviseRun
    }));
}
}  // namespace winss
