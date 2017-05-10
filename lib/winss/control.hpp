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

#ifndef LIB_WINSS_CONTROL_HPP_
#define LIB_WINSS_CONTROL_HPP_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "pipe_client.hpp"
#include "wait_multiplexer.hpp"
#include "not_owning_ptr.hpp"

namespace winss {
/**
 * Base class for inbound and outbound control item.
 */
class ControlItem {
 protected:
    const std::string name;  /**< The name of the control item. */

    /**
     * Constructs a control item with the given name.
     *
     * \param name The name of the control item.
     */
    explicit ControlItem(std::string name);

 public:
    /**
     * Gets the name of the control item.
     *
     * \return The name of the control item.
     */
    virtual const std::string& GetName() const;

    /**
     * Initializes the control item.
     */
    virtual void Init() {}

    /**
     * Signals the start of either sending events or listening to events.
     */
    virtual void Start() {}

    /**
     * Signals that the events have been sent or all have been received.
     */
    virtual bool Completed() const = 0;

    /**
     * Default destructor
     */
    virtual ~ControlItem() {}
};

/**
 * Orchestrates control.
 *
 * Control is used for the supervisor and svscan. This is to reuse the
 * orchestration logic.
 */
class Control {
 private:
    static const int kTimeoutExitCode;  /**< Timeout exit code. */
    /** The timeout group for the multiplexer. */
    static const char kTimeoutGroup[];
    /** The event multiplexer for control. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    /** The control items. */
    std::map<std::string, winss::NotOwningPtr<ControlItem>> items;
    std::set<std::string> ready;  /**< Items that are ready. */
    bool started = false;  /**< Orchestration has started. */
    const DWORD timeout;  /**< Timeout for orchestration. */
    const bool finish_all;  /**< If all control items must finish. */

 public:
    /**
     * Control constructor.
     *
     * \param multiplexer The shared multiplexer.
     * \param timeout Orchestration timeout.
     * \param finish_all If all control items must finish.
     */
    explicit Control(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        DWORD timeout = INFINITE, bool finish_all = true);
    Control(const Control&) = delete;  /**< No copy. */
    Control(Control&&) = delete;  /**< No move. */

    /**
     * Gets if control has started.
     *
     * \return True if started otherwise false.
     */
    bool IsStarted() const;

    /**
     * Add a control item.
     *
     * \param[in] item The item to add to control.
     */
    void Add(winss::NotOwningPtr<ControlItem> item);

    /**
     * Mark a control item as ready.
     *
     * \param[in] name The name of the control item.
     */
    void Ready(std::string name);

    /**
     * Remove a control item.
     *
     * \param[in] name The name of the control item.
     */
    void Remove(std::string name);

    /**
     * Starts the orchestration.
     */
    int Start();

    void operator=(const Control&) = delete;  /**< No copy. */
    Control& operator=(Control&&) = delete;  /**< No move. */
};

/**
 * Outbound control item.
 *
 * Will send a control sequence over a pipe.
 */
class OutboundControlItem :
    public ControlItem,
    public winss::PipeClientSendListener {
 private:
    static const int kNoSendExitCode;
    winss::NotOwningPtr<WaitMultiplexer> multiplexer;
    winss::NotOwningPtr<Control> control;
    winss::NotOwningPtr<winss::OutboundPipeClient> client;
    const std::vector<char>& commands;
    bool written = false;

 public:
    OutboundControlItem(winss::NotOwningPtr<WaitMultiplexer> multiplexer,
        winss::NotOwningPtr<Control> control,
        winss::NotOwningPtr<winss::OutboundPipeClient> client,
        const std::vector<char>& commands, std::string name);
    OutboundControlItem(const OutboundControlItem&) = delete;  /**< No copy. */
    OutboundControlItem(OutboundControlItem&&) = delete;  /**< No move. */

    /**
     * Initializes the outbound control item.
     */
    void Init();

    /**
     * Send the control sequence.
     */
    void Start();

    /**
     * Gets of the control item has completed sending the control sequence.
     *
     * \return True if completed otherwise false.
     */
    bool Completed() const;

    /**
     * Handler for pipe connected.
     *
     * \return True always.
     */
    bool Connected();

    /**
     * Handler for pipe write complete.
     *
     * \return True always.
     */
    bool WriteComplete();

    /**
     * Handler for pipe disconnected.
     *
     * \return True always.
     */
    bool Disconnected();

    /** No copy. */
    void operator=(const OutboundControlItem&) = delete;
    /** No move. */
    OutboundControlItem& operator=(OutboundControlItem&&) = delete;
};

/**
 * A listener for inbound control item events.
 */
class InboundControlItemListener {
 public:
    /**
     * Gets if the listener is enabled.
     *
     * \return True if enabled otherwise false.
     */
    virtual bool IsEnabled() = 0;

    /**
     * Gets if the listener can start.
     *
     * \return True if the listener can start otherwise false.
     */
    virtual bool CanStart() = 0;

    /**
     * Call back for connected event.
     */
    virtual void HandleConnected() {}

    /**
     * Call back for when a message is received.
     *
     * \return True if still waiting on messages otherwise false.
     */
    virtual bool HandleReceived(const std::vector<char>& message) = 0;

    /**
     * Default destructor
     */
    virtual ~InboundControlItemListener() {}
};

/**
 * Inbound control item.
 *
 * Will wait for the condition on the remote pipe server is met.
 */
class InboundControlItem :
    public ControlItem,
    public winss::PipeClientReceiveListener {
 private:
    winss::NotOwningPtr<WaitMultiplexer> multiplexer;
    winss::NotOwningPtr<Control> control;
    winss::NotOwningPtr<winss::InboundPipeClient> client;
    winss::NotOwningPtr<InboundControlItemListener> listener;
    bool complete = false;

 public:
    InboundControlItem(winss::NotOwningPtr<WaitMultiplexer> multiplexer,
        winss::NotOwningPtr<Control> control,
        winss::NotOwningPtr<winss::InboundPipeClient> client,
        winss::NotOwningPtr<InboundControlItemListener> listener,
        std::string name);
    InboundControlItem(const InboundControlItem&) = delete;  /**< No copy. */
    InboundControlItem(InboundControlItem&&) = delete;  /**< No move. */

    /**
     * Initializes the inbound control item.
     */
    void Init();

    /**
     * Start waiting for inbound events.
     */
    void Start();

    /**
     * Gets of the control item has completed reading the state/events.
     *
     * \return True if completed otherwise false.
     */
    bool Completed() const;

    /**
     * Handler for pipe connected.
     *
     * \return True always.
     */
    bool Connected();

     /**
     * Handler for pipe connected.
     *
     * \return True if the handler has finished otherwise false.
     */
    bool Received(const std::vector<char>& message);

    /**
     * Handler for pipe disconnected.
     *
     * \return True always.
     */
    bool Disconnected();

    /** No copy. */
    void operator=(const InboundControlItem&) = delete;
    /** No move. */
    InboundControlItem& operator=(InboundControlItem&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_CONTROL_HPP_
