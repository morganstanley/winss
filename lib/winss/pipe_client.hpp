#ifndef LIB_WINSS_PIPE_CLIENT_HPP_
#define LIB_WINSS_PIPE_CLIENT_HPP_

#include <windows.h>
#include <functional>
#include <vector>
#include "easylogging/easylogging++.hpp"
#include "wait_multiplexer.hpp"
#include "pipe_name.hpp"
#include "handle_wrapper.hpp"
#include "pipe_server.hpp"
#include "not_owning_ptr.hpp"

namespace winss {
struct PipeClientConfig {
    winss::PipeName pipe_name;
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
};
template<typename TInstance, typename TListener>
class PipeClient {
 protected:
    static const DWORD kTimeout = 5000;
    static const DWORD kBufferSize = 4096;

    bool stopping = false;

    TInstance instance;
    winss::PipeName pipe_name;
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    std::vector<winss::NotOwningPtr<TListener>> listeners;

    virtual void Triggered() {}
    virtual void Connected() {}

    void TellAll(
        const std::function<bool(TListener&)>& func) {
        auto it = listeners.begin();
        while (it != listeners.end()) {
            if (func(**it)) {
                ++it;
            } else {
                it = listeners.erase(it);
            }
        }
    }

    void Triggered(const winss::HandleWrapper& handle) {
        if (handle == instance.GetHandle()) {
            winss::OverlappedResult result = instance.GetOverlappedResult();
            if (result == REMOVE) {
                if (instance.Close()) {
                    TellAll([](TListener& listener) {
                        return listener.Disconnected();
                    });
                }
                return;
            }

            multiplexer->AddTriggeredCallback(handle, [&](
                winss::WaitMultiplexer& m, const winss::HandleWrapper& handle) {
                Triggered(handle);
            });

            if (result != SKIP) {
                Triggered();
            }
        }
    }

 public:
    explicit PipeClient(const PipeClientConfig& config) :
        pipe_name(config.pipe_name), multiplexer(config.multiplexer) {
    }

    PipeClient(const PipeClient&) = delete;
    PipeClient(PipeClient&&) = delete;

    virtual void AddListener(winss::NotOwningPtr<TListener> listener) {
        listeners.push_back(listener);
    }

    virtual bool IsStopping() const {
        return stopping;
    }

    virtual void Connect() {
        if (!stopping && !instance.IsConnected()) {
            multiplexer->AddStopCallback([&](winss::WaitMultiplexer&) {
                Stop();
            });

            if (instance.CreateFile(pipe_name)) {
                multiplexer->AddTriggeredCallback(instance.GetHandle(),
                    [&](winss::WaitMultiplexer& m,
                    const winss::HandleWrapper& handle) {
                    Triggered(handle);
                });
                if (instance.SetConnected()) {
                    Connected();
                }
            } else {
                TellAll([](TListener& listener) {
                    return listener.Disconnected();
                });
            }
        }
    }

    virtual void Stop() {
        if (!stopping) {
            stopping = true;
            instance.Closing();
        }
    }

    void operator=(const PipeClient&) = delete;
    PipeClient& operator=(PipeClient&&) = delete;

    virtual ~PipeClient() {
        if (instance.Close()) {
            TellAll([](TListener& listener) {
                return listener.Disconnected();
            });
        }
    }
};
class PipeClientConnectionListener {
 public:
    virtual bool Connected() = 0;
    virtual bool Disconnected() = 0;
    virtual ~PipeClientConnectionListener() {}
};
class PipeClientSendListener : virtual public PipeClientConnectionListener {
 public:
    virtual bool WriteComplete() = 0;
    virtual ~PipeClientSendListener() {}
};
class PipeClientRecieveListener : virtual public PipeClientConnectionListener {
 public:
    virtual bool Recieved(const std::vector<char>& message) = 0;
    virtual ~PipeClientRecieveListener() {}
};
template<typename TInstance>
class OutboundPipeClientTmpl : public PipeClient<TInstance,
    PipeClientSendListener> {
 private:
    void Connected() {
        instance.Read();

        TellAll([](winss::PipeClientSendListener& listener) {
            return listener.Connected();
        });
    }

    void Triggered() {
        if (instance.FinishWrite()) {
            instance.Write();
        } else {
            bool was_writting = instance.IsWriting();

            instance.Read();

            if (was_writting) {
                TellAll([](winss::PipeClientSendListener& listener) {
                    return listener.WriteComplete();
                });
            }
        }
    }

 public:
    explicit OutboundPipeClientTmpl(const PipeClientConfig& config) :
        winss::PipeClient<TInstance, PipeClientSendListener>
        ::PipeClient(config) {}

    OutboundPipeClientTmpl(const OutboundPipeClientTmpl&) = delete;
    OutboundPipeClientTmpl(OutboundPipeClientTmpl&&) = delete;

    virtual bool Send(const std::vector<char>& data) {
        if (instance.Queue(data)) {
            return instance.Write();
        }

        return false;
    }

    void operator=(const OutboundPipeClientTmpl&) = delete;
    OutboundPipeClientTmpl& operator=(OutboundPipeClientTmpl&&) = delete;
};
typedef OutboundPipeClientTmpl<winss::OutboundPipeInstance> OutboundPipeClient;
template<typename TInstance>
class InboundPipeClientTmpl : public PipeClient<TInstance,
    PipeClientRecieveListener> {
 private:
    bool handshake = false;

    void Connected() {
        instance.Read();
    }

    void Triggered() {
        if (instance.FinishRead()) {
            Notify();
        }

        instance.Read();
    }

    void Notify() {
        std::vector<char> buff = instance.SwapBuffer();

        if (!handshake) {
            auto pos = std::find(buff.begin(), buff.end(), 0);
            if (pos != buff.end()) {
                VLOG(6) << "Inbound pipe handshake complete";
                handshake = true;

                TellAll([](winss::PipeClientRecieveListener& listener) {
                    return listener.Connected();
                });

                buff.erase(buff.begin(), pos);
                if (buff.empty()) {
                    return;
                }
            } else {
                VLOG(1)
                    << "Inbound pipe handshake failed (expected null byte)";
                return;
            }
        }

        TellAll([buff](PipeClientRecieveListener& listener) {
            return listener.Recieved(buff);
        });
    }

 public:
    explicit InboundPipeClientTmpl(const PipeClientConfig& config) :
        winss::PipeClient<TInstance, PipeClientRecieveListener>
        ::PipeClient(config) {}

    InboundPipeClientTmpl(const InboundPipeClientTmpl&) = delete;
    InboundPipeClientTmpl(InboundPipeClientTmpl&&) = delete;

    void operator=(const InboundPipeClientTmpl&) = delete;
    InboundPipeClientTmpl& operator=(InboundPipeClientTmpl&&) = delete;
};
typedef InboundPipeClientTmpl<winss::InboundPipeInstance> InboundPipeClient;
}  // namespace winss

#endif  // LIB_WINSS_PIPE_CLIENT_HPP_
