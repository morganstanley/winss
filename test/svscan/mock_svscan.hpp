#ifndef TEST_SVSCAN_MOCK_SVSCAN_HPP_
#define TEST_SVSCAN_MOCK_SVSCAN_HPP_

#include <windows.h>
#include <filesystem>
#include "gmock/gmock.h"
#include "winss/svscan/svscan.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "mock_service.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockSvScan : public winss::SvScan {
 public:
    MockSvScan(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        const fs::path& scan_dir, DWORD rescan) :
        winss::SvScan::SvScanTmpl(multiplexer, scan_dir, rescan) {}

    MockSvScan(const MockSvScan&) = delete;
    MockSvScan(MockSvScan&&) = delete;

    MOCK_METHOD1(Scan, void(bool timeout));
    MOCK_METHOD1(CloseAllServices, void(bool ignore_flagged));
    MOCK_METHOD0(Timeout, void());
    MOCK_METHOD1(Exit, void(bool close_services));

    void operator=(const MockSvScan&) = delete;
    MockSvScan& operator=(MockSvScan&&) = delete;
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SVSCAN_HPP_
