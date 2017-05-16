/*
* Copyright 2016-2017 Morgan Stanley
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <string>
#include "gtest/gtest.h"
#include "winss/winss.hpp"
#include "winss/pipe_name.hpp"

namespace winss {
class PipeNameTest : public testing::Test {
};

TEST_F(PipeNameTest, Name) {
    winss::PipeName pipe_name1(".");
    winss::PipeName pipe_name2(".", "pipe_name2");

    const std::string& name1 = pipe_name1.Get();
    EXPECT_EQ(0, name1.find("\\\\.\\pipe\\"));
    EXPECT_GT(name1.length(), (size_t) 7);

    const std::string& name2 = pipe_name2.Get();
    EXPECT_EQ(0, name2.find("\\\\.\\pipe\\"));
    EXPECT_GT(name2.find("_pipe_name2"), (size_t) 0);
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
