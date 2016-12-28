#ifndef TEST_MOCK_PIPE_NAME_HPP_
#define TEST_MOCK_PIPE_NAME_HPP_

#include <string>
#include "winss/pipe_name.hpp"

namespace winss {
class MockPipeName : public winss::PipeName {
 public:
    explicit MockPipeName(std::string path) {
        name = path;
    }
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_NAME_HPP_
