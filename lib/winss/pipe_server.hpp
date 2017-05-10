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
/**
 * Config for a named pipe server.
 */
struct PipeServerConfig {
    winss::PipeName pipe_name;
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
};

/**
 * Base named pipe server.
 *
 * This class should be specialized as a inbound or outbound pipe server.
 *
 * \tparam TPipeInstance The named pipe instance implementation.
 */
template<typename TPipeInstance>
class PipeServer {
 protected:
    bool stopping = false;  /**< Flag if the server is stopping. */
    bool open = false;      /**< Flag if the server is open for connections. */

    /** A mapping of handles to instances. */
    std::map<winss::HandleWrapper, TPipeInstance> instances;
    /** The event multiplexer for the named pipe server. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    winss::PipeName pipe_name;  /**< The name of the pipe. */

    /**
     * Open a new named pipe for a new client to connect to.
     */
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

    /**
     * Stop the pipe server.
     */
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

    /**
     * The event handler for the pipe server.
     *
     * \param handle The handle that triggered the event.
     */
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

    /**
     * Called when a client is connected.
     *
     * \param instance The associated client instance.
     */
    virtual void Connected(TPipeInstance* instance) {}

    /**
     * Called when an event is triggered.
     *
     * \param instance The associated client instance.
     */
    virtual void Triggered(TPipeInstance* instance) {}

 public:
    /**
     * Create a new pipe instance with the given config.
     *
     * \param config The pipe server config.
     */
    explicit PipeServer(const PipeServerConfig& config) :
        pipe_name(config.pipe_name), multiplexer(config.multiplexer) {
        multiplexer->AddInitCallback([&](winss::WaitMultiplexer&) {
            StartClient();
        });

        multiplexer->AddStopCallback([&](winss::WaitMultiplexer&) {
            Stop();
        });
    }

    PipeServer(const PipeServer&) = delete;  /**< No copy. */
    PipeServer(PipeServer&&) = delete;  /**< No move. */

    /**
     * Gets if the pipe server is accepting a new connection.
     *
     * \return True if the pipe server is accepting otherwise false.
     */
    virtual bool IsAccepting() const {
        return open;
    }

    /**
     * Gets if the pipe server is stopping.
     *
     * \return True if the pipe server is stopping otherwise false.
     */
    virtual bool IsStopping() const {
        return stopping;
    }

    /**
     * Gets the current instance count.
     *
     * \return The number of instances managed by the server.
     */
    virtual size_t InstanceCount() const {
        return instances.size();
    }

    void operator=(const PipeServer&) = delete;  /**< No copy. */
    PipeServer& operator=(PipeServer&&) = delete;  /**< No move. */

    /** 
     * Disconnect all clients and close all connections.
     */
    virtual ~PipeServer() {
        for (auto it = instances.begin(); it != instances.end(); ++it) {
            it->second.DisconnectNamedPipe();
            it->second.Close();
        }
    }
};

/**
 * An outbound pipe server.
 *
 * \tparam TPipeInstance The named pipe instance implementation.
 */
template<typename TPipeInstance>
class OutboundPipeServerTmpl : public PipeServer<TPipeInstance> {
 private:
    /**
     * Called when a client is connected.
     *
     * Will send a \0 when connected to finish the handshake.
     *
     * \param instance The associated client instance.
     */
    void Connected(TPipeInstance* instance) {
        Send(instance, { 0 });  // Send null char to signal connected
    }

    /**
     * Called when an event is triggered.
     *
     * \param instance The associated client instance.
     */
    void Triggered(TPipeInstance* instance) {
        if (instance->FinishWrite()) {
            instance->Write();
        } else {
            instance->Read();
        }
    }

    /**
     * Send the given data to the specified instance.
     *
     * \param instance The instance to send to.
     * \param data The data to send.
     * \return True if the data was sent otherwise false.
     */
    bool Send(TPipeInstance* instance, const std::vector<char>& data) {
        if (instance->Queue(data)) {
            return instance->Write();
        }

        return false;
    }

 public:
    explicit OutboundPipeServerTmpl(const PipeServerConfig& config) :
        winss::PipeServer<TPipeInstance>::PipeServer(config) {}

    /** No copy. */
    OutboundPipeServerTmpl(const OutboundPipeServerTmpl&) = delete;
    /** No move. */
    OutboundPipeServerTmpl(OutboundPipeServerTmpl&&) = delete;

    /**
     * Send the given data to all instances.
     *
     * \param data The data to send.
     */
    virtual bool Send(const std::vector<char>& data) {
        bool sent = true;
        for (auto it = instances.begin(); it != instances.end(); ++it) {
            if (!Send(&it->second, data)) {
                sent = false;
            }
        }

        return sent;
    }

    /** No copy. */
    void operator=(const OutboundPipeServerTmpl&) = delete;
    /** No move. */
    OutboundPipeServerTmpl& operator=(OutboundPipeServerTmpl&&) = delete;
};

/**
 * Concrete outbound pipe server implementation.
 */
typedef OutboundPipeServerTmpl<winss::OutboundPipeInstance> OutboundPipeServer;

/**
 * A listener for server receiving data from pipe clients.
 */
class PipeServerReceiveListener {
 public:
    /**
     * Called when a client received data from a client.
     *
     * \param data The bytes that were received.
     */
    virtual bool Received(const std::vector<char>& data) = 0;

    /** Default destructor. */
    virtual ~PipeServerReceiveListener() {}
};

/**
 * An inbound pipe server.
 *
 * \tparam TPipeInstance The named pipe instance implementation.
 */
template<typename TPipeInstance>
class InboundPipeServerTmpl : public PipeServer<TPipeInstance> {
 private:
    std::vector<winss::NotOwningPtr<PipeServerReceiveListener>> listeners;

    /**
     * Called when a client is connected.
     *
     * Will start to read from the instance.
     *
     * \param instance The associated client instance.
     */
    void Connected(TPipeInstance* instance) {
        instance->Read();
    }

    /**
     * Called when an event is triggered.
     *
     * \param instance The associated client instance.
     */
    void Triggered(TPipeInstance* instance) {
        if (instance->FinishRead()) {
            Notify(instance);
        }

        instance->Read();
    }

    /**
     * Called when there is data to notify listeners of.
     */
    void Notify(TPipeInstance* instance) {
        std::vector<char> buff = instance->SwapBuffer();

        if (!buff.empty()) {
            auto it = listeners.begin();
            while (it != listeners.end()) {
                if ((*it)->Received(buff)) {
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

    /** No copy. */
    InboundPipeServerTmpl(const InboundPipeServerTmpl&) = delete;
    /** No move. */
    InboundPipeServerTmpl(InboundPipeServerTmpl&&) = delete;

    /**
     * Add a listener to the pipe server for receive events.
     */
    virtual void AddListener(
        winss::NotOwningPtr<PipeServerReceiveListener> listener) {
        listeners.push_back(listener);
    }

    /** No copy. */
    void operator=(const InboundPipeServerTmpl&) = delete;
    /** No move. */
    InboundPipeServerTmpl& operator=(InboundPipeServerTmpl&&) = delete;
};

/**
 * Concrete inbound pipe server implementation.
 */
typedef InboundPipeServerTmpl<winss::InboundPipeInstance> InboundPipeServer;
}  // namespace winss

#endif  // LIB_WINSS_PIPE_SERVER_HPP_
