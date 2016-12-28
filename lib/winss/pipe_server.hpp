#ifndef LIB_WINSS_PIPE_SERVER_HPP_
#define LIB_WINSS_PIPE_SERVER_HPP_

#include <windows.h>
#include <vector>
#include <utility>
#include <map>
#include "easylogging/easylogging++.hpp"
#include "wait_multiplexer.hpp"
#include "pipe_name.hpp"
#include "pipe_instance.hpp"
#include "handle_wrapper.hpp"
#include "not_owning_ptr.hpp"

namespace winss {
struct PipeServerConfig {
    winss::PipeName pipe_name;
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
};
template<typename TPipeInstance>
class PipeServer {
 protected:
    bool stopping = false;
    bool open = false;

    std::map<winss::HandleWrapper, TPipeInstance> instances;
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    winss::PipeName pipe_name;

    void StartClient() {
        while (!stopping && !open) {
            TPipeInstance instance;
            if (instance.CreateNamedPipe(pipe_name)) {
                winss::HandleWrapper handle = instance.GetHandle();
                instances.emplace(handle, std::move(instance));
                multiplexer->AddTriggeredCallback(handle, [&](
                    winss::WaitMultiplexer&, const winss::HandleWrapper& h) {
                    Triggered(h);
                });
                open = true;
                VLOG(6) << "Pipe server clients: " << instances.size();
            }
        }
    }

    void Stop() {
        if (!stopping) {
            stopping = true;
            for (auto it = instances.begin(); it != instances.end(); ++it) {
                if (!it->second.IsConnected()) {
                    it->second.Closing();
                }
            }
        }
    }

    void Triggered(const winss::HandleWrapper& handle) {
        auto it = instances.find(handle);
        if (it != instances.end()) {
            winss::OverlappedResult result = it->second.GetOverlappedResult();
            if (result == REMOVE) {
                if (!it->second.IsConnected()) {
                    VLOG(1) << "Pipe server client did not connect (closing)";
                    open = false;
                }
                it->second.DisconnectNamedPipe();
                it->second.Close();
                instances.erase(it);
                VLOG(6) << "Pipe server clients: " << instances.size();
                StartClient();
                return;
            }

            multiplexer->AddTriggeredCallback(handle, [&](
                winss::WaitMultiplexer& m, const winss::HandleWrapper& h) {
                Triggered(h);
            });

            if (result == SKIP) {
                return;
            }

            if (it->second.SetConnected()) {
                Connected(&it->second);
                open = false;
                StartClient();
            } else {
                Triggered(&it->second);
            }
        } else {
            VLOG(6) << "Pipe instance not found";
        }
    }

    virtual void Connected(TPipeInstance* instance) {}
    virtual void Triggered(TPipeInstance* instance) {}

 public:
    explicit PipeServer(const PipeServerConfig& config) :
        pipe_name(config.pipe_name), multiplexer(config.multiplexer) {
        multiplexer->AddInitCallback([&](winss::WaitMultiplexer&) {
            StartClient();
        });

        multiplexer->AddStopCallback([&](winss::WaitMultiplexer&) {
            Stop();
        });
    }

    PipeServer(const PipeServer&) = delete;
    PipeServer(PipeServer&&) = delete;

    virtual bool IsAccepting() const {
        return open;
    }

    virtual bool IsStopping() const {
        return stopping;
    }

    virtual size_t InstanceCount() const {
        return instances.size();
    }

    void operator=(const PipeServer&) = delete;
    PipeServer& operator=(PipeServer&&) = delete;

    virtual ~PipeServer() {
        for (auto it = instances.begin(); it != instances.end(); ++it) {
            it->second.DisconnectNamedPipe();
            it->second.Close();
        }
    }
};
template<typename TPipeInstance>
class OutboundPipeServerTmpl : public PipeServer<TPipeInstance> {
 private:
    void Connected(TPipeInstance* instance) {
        Send(instance, { 0 });  // Send null char to signal connected
    }

    void Triggered(TPipeInstance* instance) {
        if (instance->FinishWrite()) {
            instance->Write();
        } else {
            instance->Read();
        }
    }

    bool Send(TPipeInstance* instance, const std::vector<char>& data) {
        if (instance->Queue(data)) {
            return instance->Write();
        }

        return false;
    }

 public:
    explicit OutboundPipeServerTmpl(const PipeServerConfig& config) :
        winss::PipeServer<TPipeInstance>::PipeServer(config) {}

    virtual bool Send(const std::vector<char>& data) {
        bool sent = true;
        for (auto it = instances.begin(); it != instances.end(); ++it) {
            if (!Send(&it->second, data)) {
                sent = false;
            }
        }

        return sent;
    }
};
typedef OutboundPipeServerTmpl<winss::OutboundPipeInstance> OutboundPipeServer;
class PipeServerRecieveListener {
 public:
    virtual bool Recieved(const std::vector<char>& data) = 0;
    virtual ~PipeServerRecieveListener() {}
};
template<typename TPipeInstance>
class InboundPipeServerTmpl : public PipeServer<TPipeInstance> {
 private:
    std::vector<winss::NotOwningPtr<PipeServerRecieveListener>> listeners;

    void Connected(TPipeInstance* instance) {
        instance->Read();
    }

    void Triggered(TPipeInstance* instance) {
        if (instance->FinishRead()) {
            Notify(instance);
        }

        instance->Read();
    }

    void Notify(TPipeInstance* instance) {
        std::vector<char> buff = instance->SwapBuffer();

        if (!buff.empty()) {
            auto it = listeners.begin();
            while (it != listeners.end()) {
                if ((*it)->Recieved(buff)) {
                    ++it;
                } else {
                    it = listeners.erase(it);
                }
            }
        }
    }

 public:
    explicit InboundPipeServerTmpl(const PipeServerConfig& config) :
        winss::PipeServer<TPipeInstance>::PipeServer(config) {}

    virtual void AddListener(
        winss::NotOwningPtr<PipeServerRecieveListener> listener) {
        listeners.push_back(listener);
    }
};
typedef InboundPipeServerTmpl<winss::InboundPipeInstance> InboundPipeServer;
}  // namespace winss

#endif  // LIB_WINSS_PIPE_SERVER_HPP_
