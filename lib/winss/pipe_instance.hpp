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

#ifndef LIB_WINSS_PIPE_INSTANCE_HPP_
#define LIB_WINSS_PIPE_INSTANCE_HPP_

#include <windows.h>
#include <vector>
#include <queue>
#include "handle_wrapper.hpp"
#include "pipe_name.hpp"

namespace winss {
/**
 * The result of the overlapped operation.
 */
enum OverlappedResult {
    REMOVE,    /**< Close client. */
    CONTINUE,  /**< CLient is ready. */
    SKIP       /**< Wait till next result. */
};

/**
 * The pipe instance which is shared between client and server.
 */
class PipeInstance {
 protected:
    OVERLAPPED overlapped;     /**< The overlapped structure. */
    HANDLE handle = nullptr;   /**< The event handle. */
    bool connected = false;    /**< Flags if the instance is connected. */
    bool pending_io = false;   /**< Flagged if IO event is pending. */
    bool close = false;        /**< Flagged if instance is closing. */
    std::vector<char> buffer;  /**< The instance byte buffer. */
    DWORD bytes = 0;           /**< The bytes read or written. */

    /**
     * Checks if error is a real error or pending IO operation.
     *
     * \return False if the IO event is not pending otherwise true.
     */
    bool CheckError();

 public:
    static const DWORD kBufferSize = 4096;  /**< The pipe buffer. */
    static const DWORD kTimeout = 5000;     /**< The pipe timeout. */

    /**
     * Creates a new pipe instance.
     */
    PipeInstance();

    PipeInstance(const PipeInstance&) = delete;  /**< No copy. */

    /**
     * Create a new pipe instance from another.
     *
     * \param instance The pipe instance to move from.
     */
    PipeInstance(PipeInstance&& instance);

    /**
     * Gets if there is pending IO operations.
     *
     * \return True if pending IO otherwise false.
     */
    virtual bool IsPendingIO() const;

    /**
     * Gets if the instance is connected.
     *
     * \return True if connected otherwise false.
     */
    virtual bool IsConnected() const;

    /**
     * Gets if the instance is closing.
     *
     * \return True if the instance is closing otherwise false.
     */
    virtual bool IsClosing() const;

    /**
     * Gets the event handle for the instance.
     *
     * \return The handle wrapper of the event.
     * \see HandleWrapper
     */
    virtual winss::HandleWrapper GetHandle() const;

    /**
     * Sets the instance to connected state.
     *
     * \return True if the connected state was updated otherwise false.
     */
    virtual bool SetConnected();

    /**
     * Gets the last overlapped result of the instance.
     *
     * \return An overlapped result for the last IO operation.
     */
    virtual OverlappedResult GetOverlappedResult();

    /**
     * Creates the Windows named pipe server.
     *
     * \param pipe_name The name of the pipe.
     * \return True if the pipe server was created otherwise false.
     */
    virtual bool CreateNamedPipe(const winss::PipeName& pipe_name);

    /**
     * Creates a Windows named pipe client.
     *
     * \param pipe_name The name of the pipe.
     * \return True if pipe client was created otherwise false.
     */
    virtual bool CreateFile(const winss::PipeName& pipe_name);

    /**
     * Marks the pipe instance as closing and triggers the event.
     *
     * The client should close the pipe in the event handler.
     */
    virtual void Closing();

    /**
     * DIsconnect the client from the pipe server.
     */
    virtual void DisconnectNamedPipe();

    /**
     * Close the pipe connection.
     *
     * \return True if the close changed the state otherwise false.
     */
    virtual bool Close();

    /** No copy. */
    PipeInstance& operator=(const PipeInstance&) = delete;

    /**
     * Move a pipe instance to this instance.
     *
     * \param instance The pipe instance to move from.
     * \return This pipe instance.
     */
    PipeInstance& operator=(PipeInstance&& instance);

    /** Default destructor. */
    virtual ~PipeInstance();
};

/**
 * A pipe instance for outbound client/server.
 */
class OutboundPipeInstance : public PipeInstance {
 private:
    bool writting = false;  /**< Flags if writing. */
    std::queue<std::vector<char>> message_queue;  /**< The message queue. */

 public:
    /**
     * Creates an outbound pipe instance.
     */
    OutboundPipeInstance();

    /** No copy. */
    OutboundPipeInstance(const OutboundPipeInstance&) = delete;

    /**
     * Create a new outbound pipe instance and move from another
     *
     * \param instance The outbound pipe instance to move from.
     */
    OutboundPipeInstance(OutboundPipeInstance&& instance);

    /**
     * Queue the data to be sent.
     *
     * \param data The data to queue.
     */
    bool Queue(const std::vector<char>& data);

    /**
     * Get if the instance has messages to send.
     *
     * \return True if there is messages to send otherwise false.
     */
    bool HasMessages() const;

    /**
     * Get if the instance is currently writing data.
     *
     * \return True if writing data to the client otherwise false.
     */
    bool IsWriting() const;

    /**
     * Complete writing data.
     *
     * \return True if there is more data to send otherwise false.
     */
    bool FinishWrite();

    /**
     * Write the data.
     *
     * \return True if the data was written otherwise false.
     */
    bool Write();

    /**
     * Read for write responses.
     */
    void Read();

    /** No copy. */
    OutboundPipeInstance& operator=(const OutboundPipeInstance&) = delete;

    /**
     * Move an outbound pipe instance to this instance.
     *
     * \param instance The outbound pipe instance to move from.
     * \return This outbound pipe instance.
     */
    OutboundPipeInstance& operator=(OutboundPipeInstance&& instance);
};

/**
 * A pipe instance for inbound client/server.
 */
class InboundPipeInstance : public PipeInstance {
 public:
    /**
     * Creates a new outbound pipe instance.
     */
    InboundPipeInstance();

    /** No copy. */
    InboundPipeInstance(const InboundPipeInstance&) = delete;

    /**
     * Create a new inbound pipe instance and move from another.
     *
     * \param instance The inbound pipe instance to move from.
     */
    InboundPipeInstance(InboundPipeInstance&& instance);

    /**
     * Finishes reading from the instance.
     *
     * \return True if bytes were read otherwise false.
     */
    bool FinishRead();

    /**
     * Read from a pipe instance for events and data.
     */
    void Read();

    /**
     * Swap the read buffer to be used by a listener.
     *
     * \return The old buffer.
     */
    std::vector<char> SwapBuffer();

    /** No copy. */
    InboundPipeInstance& operator=(const InboundPipeInstance&) = delete;

    /**
     * Move an inbound pipe instance to this instance.
     *
     * \param instance The inbound pipe instance to move from.
     * \return This inbound pipe instance.
     */
    InboundPipeInstance& operator=(InboundPipeInstance&& instance);
};
}  // namespace winss

#endif  // LIB_WINSS_PIPE_INSTANCE_HPP_
