#ifndef TEST_MOCK_INTERFACE_HPP_
#define TEST_MOCK_INTERFACE_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <memory>

using ::testing::NiceMock;

namespace winss {
template<typename TInterface>
class MockInterface {
 private:
    std::shared_ptr<NiceMock<TInterface>> instance;
 public:
    MockInterface() : instance(std::make_shared<NiceMock<TInterface>>()) {
        TInterface::instance = instance;
    }

    TInterface& operator*() const {
        return instance.operator*();
    }

    TInterface* operator->() const {
        return instance.operator->();
    }

    ~MockInterface() {
        TInterface::instance = nullptr;
    }
};
}  // namespace winss

#endif  // TEST_MOCK_INTERFACE_HPP_
