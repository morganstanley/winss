#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/supervise/state_file.hpp"
#include "winss/supervise/supervise.hpp"
#include "../mock_interface.hpp"
#include "../mock_filesystem_interface.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Throw;

namespace winss {
class SuperviseStateFileTest : public testing::Test {
};

TEST_F(SuperviseStateFileTest, Notify) {
    MockInterface<winss::MockFilesystemInterface> file;
    winss::SuperviseState state{};
    state.is_run_process = true;
    state.is_up = true;
    state.initially_up = true;

    winss::SuperviseStateFile state_file("test");

    EXPECT_CALL(*file, Write(_, _)).Times(1);
    EXPECT_TRUE(state_file.Notify(START, state));
}

TEST_F(SuperviseStateFileTest, NotifyException) {
    MockInterface<winss::MockFilesystemInterface> file;
    winss::SuperviseState state{};
    state.is_run_process = true;
    state.is_up = true;
    state.initially_up = true;

    winss::SuperviseStateFile state_file("test");

    EXPECT_CALL(*file, Write(_, _)).WillOnce(Throw(std::exception("error")));
    // Always true because listener will still listen
    EXPECT_TRUE(state_file.Notify(START, state));
}

TEST_F(SuperviseStateFileTest, Read) {
    MockInterface<winss::MockFilesystemInterface> file;
    winss::SuperviseState state{};

    winss::SuperviseStateFile state_file("test");

    EXPECT_CALL(*file, Read(_)).WillOnce(Return(
        "{\"count\":1,\"exit\":256,\"initial\":\"up\","
        "\"last\":\"2016-10-19T19:13:42\",\"pid\":0,"
        "\"proc\":\"finish\",\"remaining\":0,\"state\":\"down\","
        "\"time\":\"2016-10-19T19:13:42\"}"));
    EXPECT_TRUE(state_file.Read(&state));
    EXPECT_TRUE(state.initially_up);
    EXPECT_FALSE(state.is_up);
    EXPECT_FALSE(state.is_run_process);
    EXPECT_EQ(256, state.exit_code);
}

TEST_F(SuperviseStateFileTest, ReadException) {
    MockInterface<winss::MockFilesystemInterface> file;
    winss::SuperviseState state{};

    winss::SuperviseStateFile state_file("test");

    EXPECT_CALL(*file, Read(_)).WillOnce(Throw(std::exception("error")));
    EXPECT_FALSE(state_file.Read(&state));
}

TEST_F(SuperviseStateFileTest, Format) {
    MockInterface<winss::MockFilesystemInterface> file;
    winss::SuperviseState state{};
    state.is_run_process = true;
    state.is_up = true;
    state.initially_up = true;
    state.pid = 15;
    state.up_count = 2;
    state.remaining_count = 0;

    winss::SuperviseStateFile state_file("test");

    auto formatted = state_file.Format(state, true);
    EXPECT_NE(std::string::npos, formatted.find("up"));
    EXPECT_NE(std::string::npos, formatted.find("want down"));

    state.initially_up = false;
    state.remaining_count = -1;

    formatted = state_file.Format(state, true);
    EXPECT_NE(std::string::npos, formatted.find("up"));
    EXPECT_NE(std::string::npos, formatted.find("normally down"));

    state.is_up = false;
    state.exit_code = 256;

    formatted = state_file.Format(state, true);
    EXPECT_NE(std::string::npos, formatted.find("down"));
    EXPECT_NE(std::string::npos, formatted.find("256"));

    state.exit_code = 256;

    formatted = state_file.Format(state, true);
    EXPECT_NE(std::string::npos, formatted.find("down"));
    EXPECT_NE(std::string::npos, formatted.find("want up"));

    state.remaining_count = 0;
    state.initially_up = true;

    formatted = state_file.Format(state, true);
    EXPECT_NE(std::string::npos, formatted.find("down"));
    EXPECT_NE(std::string::npos, formatted.find("normally up"));
    EXPECT_NE(std::string::npos, formatted.find("paused"));

    formatted = state_file.Format(state, false);
    EXPECT_NE(std::string::npos, formatted.find("down"));
    EXPECT_EQ(std::string::npos, formatted.find("paused"));
}
}  // namespace winss
