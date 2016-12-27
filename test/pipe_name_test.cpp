#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <winss/pipe_name.hpp>
#include <string>
#include <utility>

namespace winss {
class PipeNameTest : public testing::Test {
};

TEST_F(PipeNameTest, Name) {
    winss::PipeName pipe_name1(".");
    winss::PipeName pipe_name2(".", "pipe_name2");

    const std::string& name1 = pipe_name1.Get();
    EXPECT_EQ(0, name1.find("\\\\.\\pipe\\"));
    EXPECT_GT(name1.length(), 7);

    const std::string& name2 = pipe_name2.Get();
    EXPECT_EQ(0, name2.find("\\\\.\\pipe\\"));
    EXPECT_GT(name2.find("_pipe_name2"), 0);
}

TEST_F(PipeNameTest, Append) {
    winss::PipeName pipe_name1(".");

    winss::PipeName pipe_name2 = pipe_name1.Append("test");

    EXPECT_EQ(pipe_name1.Get() + "_test", pipe_name2.Get());

    winss::PipeName pipe_name3 = pipe_name2.Append("");

    EXPECT_EQ(pipe_name2.Get(), pipe_name3.Get());
}

TEST_F(PipeNameTest, CopyAndMove) {
    winss::PipeName pipe_name1(".");

    winss::PipeName pipe_name2(pipe_name1);

    EXPECT_EQ(pipe_name1.Get(), pipe_name2.Get());

    winss::PipeName pipe_name3(std::move(pipe_name2));

    EXPECT_EQ("", pipe_name2.Get());
    EXPECT_EQ(pipe_name1.Get(), pipe_name3.Get());

    winss::PipeName pipe_name4("..");
    pipe_name4 = pipe_name1;

    EXPECT_EQ(pipe_name1.Get(), pipe_name4.Get());

    pipe_name2 = std::move(pipe_name4);
    EXPECT_EQ("", pipe_name4.Get());
    EXPECT_EQ(pipe_name1.Get(), pipe_name2.Get());
}
}  // namespace winss
