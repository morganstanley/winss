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

#ifndef LIB_WINSS_PROCESS_HPP_
#define LIB_WINSS_PROCESS_HPP_

#include <windows.h>
#include <string>
#include "handle_wrapper.hpp"
#include "environment.hpp"

namespace winss {
/**
 * Parameters to start a Windows process.
 */
struct ProcessParams {
    std::string cmd;    /**< The command and arguments. */
    bool create_group;  /**< Create a new process group or not. */
    std::string dir;    /**< Start the process in this directory. */
    winss::HandleWrapper stdout_pipe;  /**< STDOUT pipe. */
    winss::HandleWrapper stderr_pipe;  /**< STDERR pipe. */
    winss::HandleWrapper stdin_pipe;   /**< STDIN pipe. */
    winss::Environment* env;  /**< The process environment. */
};

/**
 * Manages the life cycle of a process.
  */
class Process {
 private:
    PROCESS_INFORMATION proc_info;  /**< Low level process info. */

 public:
    /**
     * Create a empty process.
     */
    Process();

    /** No copy. */
    Process(const Process&) = delete;

    /**
     * Create an empty process and move the management of another to this one.
     *
     * \param p The other process.
     */
    Process(Process&& p);

    /**
     * Get current process ID.
     *
     * \return The process ID.
     */
    virtual DWORD GetProcessId() const;

    /**
     * Get the exit code of the process.
     *
     * \return The process exit code.
     */
    virtual DWORD GetExitCode() const;

    /**
     * Gets if the process is created.
     *
     * \return True if the process is created otherwise false.
     */
    virtual bool IsCreated() const;

    /**
     * Gets if the process is active.
     *
     * \return True if the process is active otherwise false.
     */
    virtual bool IsActive() const;

    /**
     * Gets the process handle.
     *
     * \return A handle wrapper to the process handle.
     * \see HandleWraper.
     */
    virtual winss::HandleWrapper GetHandle() const;

    /**
     * Create the process given the parameters.
     *
     * \param params The process parameters.
     * \return True id the process was created otherwise false.
     */
    virtual bool Create(const ProcessParams& params);

    /**
     * Send a CTRL+BREAK to the process.
     *
     * The process needs to be created in a new group for this to work.
     */
    virtual void SendBreak();

    /**
     * Terminate the process.
     *
     * There is no graceful handlers for this type of termination.
     */
    virtual void Terminate();

    /**
     * Close the handle to the process but leaves it running.
     */
    virtual void Close();

    /** No copy. */
    void operator=(const Process&) = delete;

    /**
     * Move management of another process to this one.
     *
     * \param p The other process.
     */
    Process& operator=(Process&& p);

    /**
     * Cleans up the process.
     */
    ~Process();
};
}  // namespace winss

#endif  // LIB_WINSS_PROCESS_HPP_
