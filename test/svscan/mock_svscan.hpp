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
        winss::SvScan::SvScanTmpl(multiplexer, scan_dir, rescan, false,
            winss::EventWrapper()) {}

    MockSvScan(const MockSvScan&) = delete;
    MockSvScan(MockSvScan&&) = delete;

    MOCK_METHOD1(Scan, void(bool timeout));
    MOCK_METHOD1(CloseAllServices, void(bool ignore_flagged));
    MOCK_METHOD0(Timeout, void());
    MOCK_METHOD1(Exit, void(bool close_services));

    MockSvScan& operator=(const MockSvScan&) = delete;
    MockSvScan& operator=(MockSvScan&&) = delete;
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SVSCAN_HPP_
