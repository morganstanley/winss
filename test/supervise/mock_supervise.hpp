#ifndef TEST_SUPERVISE_MOCK_SUPERVISE_HPP_
#define TEST_SUPERVISE_MOCK_SUPERVISE_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/not_owning_ptr.hpp>
#include <winss/wait_multiplexer.hpp>
#include <winss/handle_wrapper.hpp>
#include <winss/supervise/supervise.hpp>
#include <filesystem>

namespace fs = std::experimental::filesystem;

namespace winss {
class MockSupervise : public winss::Supervise {
 public:
    MockSupervise(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        const fs::path& service_dir) :
        winss::Supervise::SuperviseTmpl(multiplexer, service_dir) {}

    MockSupervise(const MockSupervise&) = delete;
    MockSupervise(MockSupervise&&) = delete;

    MOCK_CONST_METHOD0(GetState, const SuperviseState&());
    MOCK_METHOD1(AddListener, bool(winss::SuperviseListener* listener));
    MOCK_METHOD0(Up, void());
    MOCK_METHOD0(Once, void());
    MOCK_METHOD0(OnceAtMost, void());
    MOCK_METHOD0(Down, void());
    MOCK_METHOD0(Kill, void());
    MOCK_METHOD0(Term, void());
    MOCK_METHOD0(Exit, void());

    void operator=(const MockSupervise&) = delete;
    MockSupervise& operator=(MockSupervise&&) = delete;
};
class MockSuperviseListener : public winss::SuperviseListener {
 public:
    MOCK_METHOD2(Notify, bool(SuperviseNotification notification,
        const SuperviseState& state));
};
}  // namespace winss

#endif  // TEST_SUPERVISE_MOCK_SUPERVISE_HPP_
