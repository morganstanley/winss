#ifndef TEST_MOCK_CONTROL_HPP_
#define TEST_MOCK_CONTROL_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/control.hpp>
#include <vector>

namespace winss {
class MockInboundControlItemListener
    : public winss::InboundControlItemListener {
 public:
    MOCK_METHOD0(IsEnabled, bool());
    MOCK_METHOD0(CanStart, bool());
    MOCK_METHOD0(HandleConnected, void());
    MOCK_METHOD1(HandleRecieved, bool(const std::vector<char>& message));
};
}  // namespace winss

#endif  // TEST_MOCK_CONTROL_HPP_
