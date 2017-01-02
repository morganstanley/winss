#include "event_wrapper.hpp"
#include "windows_interface.hpp"
#include "handle_wrapper.hpp"

winss::EventWrapper::EventWrapper() {
    handle = WINDOWS.CreateEvent(nullptr, true, false, nullptr);
}

bool winss::EventWrapper::IsSet() const {
    return WINDOWS.WaitForSingleObject(handle, 0) != WAIT_TIMEOUT;
}

bool winss::EventWrapper::Set() {
    return WINDOWS.SetEvent(handle);
}

winss::HandleWrapper winss::EventWrapper::GetHandle() const {
    return winss::HandleWrapper(handle, false, SYNCHRONIZE);
}

winss::EventWrapper::~EventWrapper() {
    WINDOWS.CloseHandle(handle);
}
