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
/**
 * Config for a named pipe client.
 */
struct PipeClientConfig {
    winss::PipeName pipe_name;  /**< The name of the named pipe. */
    /** The event multiplexer for the named pipe client. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
};

/**
 * Base named pipe client.
 *
 * This class should be specialized as a inbound or outbound pipe client.
 *
 * \tparam TInstance The named pipe instance implementation.
 * \tparam TListener The type of pipe listener.
 */
template<typename TInstance, typename TListener>
class PipeClient {
 protected:
    bool stopping = false;  /** Marked if stopping the client. */

    TInstance instance;  /** The pipe instance. */
    winss::PipeName pipe_name;
    /** The event multiplexer for the pipe client. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    /** Listeners for the pipe client. */
    std::vector<winss::NotOwningPtr<TListener>> listeners;

    /**
     * Called when an event is triggered.
     */
    virtual void Triggered() {}

    /**
     * Called when the client is connected.
     */
    virtual void Connected() {}

    /**
     * Call a function against all listeners.
     *
     * \param func The function to invoke against all listeners.
     */
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

    /**
     * Event handler for the pipe client.
     *
     * \param handle The handle which triggered the event.
     */
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

            multiplexer->AddTriggeredCallback(handle, [this](
                winss::WaitMultiplexer&, const winss::HandleWrapper& handle) {
                Triggered(handle);
            });

            if (result != SKIP) {
                Triggered();
            }
        }
    }

 public:
    /**
     * Creates a pipe client with the given config.
     *
     * \param config The pipe client confog.
     */
    explicit PipeClient(const PipeClientConfig& config) :
        pipe_name(config.pipe_name), multiplexer(config.multiplexer) {
    }

    PipeClient(const PipeClient&) = delete;  /**< No copy. */
    PipeClient(PipeClient&&) = delete;  /**< No move. */

    /**
     * Add a listener to the client.
     *
     * \param listener The pipe client listener.
     */
    virtual void AddListener(winss::NotOwningPtr<TListener> listener) {
        listeners.push_back(listener);
    }

    /**
     * Gets if the pipe client is stopping.
     *
     * \return True if the pipe client is stopping otherwise false.
     */
    virtual bool IsStopping() const {
        return stopping;
    }

    /**
     * Start the connection process to the pipe server.
     */
    virtual void Connect() {
        if (!stopping && !instance.IsConnected()) {
            multiplexer->AddStopCallback([this](winss::WaitMultiplexer&) {
                this->Stop();
            });

            if (instance.CreateFile(pipe_name)) {
                multiplexer->AddTriggeredCallback(instance.GetHandle(),
                    [this](winss::WaitMultiplexer&,
                        const winss::HandleWrapper& handle) {
                    this->Triggered(handle);
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

    /**
     * Stop the pipe client.
     */
    virtual void Stop() {
        if (!stopping) {
            stopping = true;
            instance.Closing();
        }
    }

    PipeClient& operator=(const PipeClient&) = delete;  /**< No copy. */
    PipeClient& operator=(PipeClient&&) = delete;  /**< No move. */

    /**
     * Close the pipe client and notify listeners.
     */
    virtual ~PipeClient() {
        if (instance.Close()) {
            TellAll([](TListener& listener) {
                return listener.Disconnected();
            });
        }
    }
};

/**
 * A listener for pipe client connection events.
 */
class PipeClientConnectionListener {
 public:
    /**
     * Called when the pipe client is connected.
     *
     * \return True if still listening for events otherwise false.
     */
    virtual bool Connected() = 0;

    /**
     * Called when the pipe client is disconnected.
     *
     * \return True if still listening for events otherwise false.
     */
    virtual bool Disconnected() = 0;

    /** Default destructor. */
    virtual ~PipeClientConnectionListener() {}
};

/**
 * A listener for pipe client send complete events.
 */
class PipeClientSendListener : virtual public PipeClientConnectionListener {
 public:
    /**
     * Called when the pipe client has finished sending data.
     *
     * \return True if still listening for events otherwise false.
     */
    virtual bool WriteComplete() = 0;

    /** Default destructor. */
    virtual ~PipeClientSendListener() {}
};

/**
 * A listener for pipe client received data events.
 */
class PipeClientReceiveListener : virtual public PipeClientConnectionListener {
 public:
    /**
     * Called when the pipe client has received data.
     *
     * \param message The list of bytes that was received.
     * \return True if still listening for events otherwise false.
     */
    virtual bool Received(const std::vector<char>& message) = 0;

    /** Default destructor. */
    virtual ~PipeClientReceiveListener() {}
};

/**
 * An outbound pipe client.
 *
 * \tparam TInstance The named pipe instance implementation.
 */
template<typename TInstance>
class OutboundPipeClientTmpl : public PipeClient<TInstance,
    PipeClientSendListener> {
 private:
    /**
     * Called when the pipe client is connected.
     */
    void Connected() {
        instance.Read();

        TellAll([](winss::PipeClientSendListener& listener) {
            return listener.Connected();
        });
    }

     /**
     * Called when a pipe client event has been triggered.
     */
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
    /**
     * Creates an outbound pipe client with the given config.
     *
     * \param config The pipe client confog.
     */
    explicit OutboundPipeClientTmpl(const PipeClientConfig& config) :
        winss::PipeClient<TInstance, PipeClientSendListener>
        ::PipeClient(config) {}

    /** No copy. */
    OutboundPipeClientTmpl(const OutboundPipeClientTmpl&) = delete;
    /** No move. */
    OutboundPipeClientTmpl(OutboundPipeClientTmpl&&) = delete;

    /**
     * Sends the given list of bytes to the pipe server.
     *
     * If the client is in the process of sending data then it will be
     * queued until it is free to write.
     *
     * \return True if the data was written otherwise false.
     */
    virtual bool Send(const std::vector<char>& data) {
        if (instance.Queue(data)) {
            return instance.Write();
        }

        return false;
    }

    /** No copy. */
    OutboundPipeClientTmpl& operator=(const OutboundPipeClientTmpl&) = delete;
    /** No move. */
    OutboundPipeClientTmpl& operator=(OutboundPipeClientTmpl&&) = delete;
};

/**
 * A concrete outbound pipe client.
 */
typedef OutboundPipeClientTmpl<winss::OutboundPipeInstance> OutboundPipeClient;

/**
 * An inbound pipe client.
 *
 * The client will connect to the server and wait for a \0 before starting
 * to read data. This is because there are no connected events for reading
 * and might miss some data.
 *
 * \tparam TInstance The named pipe instance implementation.
 */
template<typename TInstance>
class InboundPipeClientTmpl : public PipeClient<TInstance,
    PipeClientReceiveListener> {
 private:
    bool handshake = false;  /**< If the client is doing a handshake. */

    /**
     * Called when the pipe client is connected.
     */
    void Connected() {
        instance.Read();
    }

    /**
     * Called when a pipe client event has been triggered.
     */
    void Triggered() {
        if (instance.FinishRead()) {
            Notify();
        }

        instance.Read();
    }

    /**
     * Called when there is data to notify listeners of.
     */
    void Notify() {
        std::vector<char> buff = instance.SwapBuffer();

        if (!handshake) {
            auto pos = std::find(buff.begin(), buff.end(), 0);
            if (pos != buff.end()) {
                VLOG(6) << "Inbound pipe handshake complete";
                handshake = true;

                TellAll([](winss::PipeClientReceiveListener& listener) {
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

        TellAll([buff](PipeClientReceiveListener& listener) {
            return listener.Received(buff);
        });
    }

 public:
    /**
     * Creates an inbound pipe client with the given config.
     *
     * \param config The pipe client confog.
     */
    explicit InboundPipeClientTmpl(const PipeClientConfig& config) :
        winss::PipeClient<TInstance, PipeClientReceiveListener>
        ::PipeClient(config) {}

    /** No copy. */
    InboundPipeClientTmpl(const InboundPipeClientTmpl&) = delete;
    /** No move. */
    InboundPipeClientTmpl(InboundPipeClientTmpl&&) = delete;

    /** No copy. */
    InboundPipeClientTmpl& operator=(const InboundPipeClientTmpl&) = delete;
    /** No move. */
    InboundPipeClientTmpl& operator=(InboundPipeClientTmpl&&) = delete;
};

/**
 * A concrete inbound pipe client.
 */
typedef InboundPipeClientTmpl<winss::InboundPipeInstance> InboundPipeClient;
}  // namespace winss

#endif  // LIB_WINSS_PIPE_CLIENT_HPP_
