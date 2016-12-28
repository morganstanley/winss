#include <filesystem>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/log/log.hpp"
#include "winss/not_owning_ptr.hpp"
#include "../mock_interface.hpp"
#include "../mock_filesystem_interface.hpp"
#include "../mock_path_mutex.hpp"
#include "mock_log_stream_wrapper.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class LogTest : public testing::Test {
};
class MockedLog : public LogTmpl<winss::MockPathMutex> {
 public:
    MockedLog(winss::NotOwningPtr<winss::MockLogStreamReader> reader,
        winss::NotOwningPtr<winss::MockLogStreamWriter> writer,
        const winss::LogSettings& settings) :
        winss::LogTmpl<winss::MockPathMutex>::LogTmpl(reader, writer,
            settings) {}

    winss::MockPathMutex* GetMutex() {
        return &mutex;
    }
};

TEST_F(LogTest, DirectoryDoesNotExist) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockLogStreamReader> reader;
    NiceMock<winss::MockLogStreamWriter> writer;

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(false));

    winss::LogSettings settings{};

    winss::MockedLog log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        settings);
    EXPECT_EQ(winss::MockedLog::kFatalExitCode, log.Start());
}

TEST_F(LogTest, MutexTaken) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockLogStreamReader> reader;
    NiceMock<winss::MockLogStreamWriter> writer;

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(true));

    winss::LogSettings settings{};

    winss::MockedLog log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        settings);
    EXPECT_CALL(*log.GetMutex(), Lock()).WillOnce(Return(false));

    EXPECT_EQ(winss::MockedLog::kMutexTaken, log.Start());
}

TEST_F(LogTest, CannotOpenLogFile) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockLogStreamReader> reader;
    NiceMock<winss::MockLogStreamWriter> writer;

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(true));
    EXPECT_CALL(writer, Open(_)).WillOnce(Return(false));

    winss::LogSettings settings{};

    winss::MockedLog log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        settings);
    EXPECT_CALL(*log.GetMutex(), Lock()).WillOnce(Return(true));

    EXPECT_EQ(winss::MockedLog::kFatalExitCode, log.Start());
}

TEST_F(LogTest, Simple) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockLogStreamReader> reader;
    NiceMock<winss::MockLogStreamWriter> writer;

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(true));

    EXPECT_CALL(reader, IsEOF())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(reader, GetLine())
        .WillOnce(Return("This is a test"));

    EXPECT_CALL(writer, Open(_)).WillOnce(Return(true));
    EXPECT_CALL(writer, GetPos()).WillOnce(Return(0));
    EXPECT_CALL(writer, Write("This is a test")).Times(1);

    winss::LogSettings settings{};

    winss::MockedLog log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        settings);
    EXPECT_CALL(*log.GetMutex(), Lock()).WillOnce(Return(true));

    EXPECT_EQ(0, log.Start());
}

TEST_F(LogTest, Rotate) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockLogStreamReader> reader;
    NiceMock<winss::MockLogStreamWriter> writer;

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, GetFiles(_)).WillOnce(Return(std::vector<fs::path>{
        "@14768836971725029.u", "@14768836971725028.u", "log", "current"
    }));
    EXPECT_CALL(*file, Remove(fs::path("@14768836971725028.u")))
        .WillOnce(Return(true));

    EXPECT_CALL(reader, IsEOF())
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(reader, GetLine())
        .WillOnce(Return("This is a test"));

    EXPECT_CALL(writer, Open(_))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(writer, GetPos()).WillOnce(Return(5));
    EXPECT_CALL(writer, Write("This is a test")).Times(1);

    winss::LogSettings settings{};
    settings.file_size = 1;
    settings.number = 1;

    winss::MockedLog log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        settings);
    EXPECT_CALL(*log.GetMutex(), Lock()).WillOnce(Return(true));

    EXPECT_EQ(0, log.Start());
}
}  // namespace winss
