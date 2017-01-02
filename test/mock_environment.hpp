#ifndef TEST_MOCK_ENVIRONMENT_HPP_
#define TEST_MOCK_ENVIRONMENT_HPP_

#include <vector>
#include "gmock/gmock.h"
#include "winss/environment.hpp"

namespace winss {
class MockEnviornment : public winss::Environment {
 public:
    MOCK_METHOD0(ReadEnv, std::vector<char>());
};
}  // namespace winss

#endif  // TEST_MOCK_ENVIRONMENT_HPP_
