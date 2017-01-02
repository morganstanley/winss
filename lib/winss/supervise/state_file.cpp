#define NOMINMAX
#include "state_file.hpp"
#include <filesystem>
#include <sstream>
#include <chrono>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../filesystem_interface.hpp"
#include "../utils.hpp"
#include "json/json.hpp"
#include "supervise.hpp"

namespace fs = std::experimental::filesystem;

const char winss::SuperviseStateFile::kStateFile[] = "state";

winss::SuperviseStateFile::SuperviseStateFile(fs::path service_dir) :
    state_file(service_dir /
        fs::path(winss::Supervise::kMutexName) / fs::path(kStateFile)) {}

const fs::path& winss::SuperviseStateFile::GetPath() const {
    return state_file;
}

bool winss::SuperviseStateFile::Notify(
    winss::SuperviseNotification notification,
    const winss::SuperviseState& state) {

    try {
        nlohmann::json json = {
            { "time", winss::Utils::ConvertToISOString(state.time) },
            { "last", winss::Utils::ConvertToISOString(state.last) },
            { "proc", state.is_run_process ?
                winss::Supervise::kRunFile :
                winss::Supervise::kFinishFile },
            { "state", state.is_up ? "up" : "down" },
            { "initial", state.initially_up ? "up" : "down" },
            { "count", state.up_count },
            { "remaining", state.remaining_count },
            { "pid", state.pid },
            { "exit", state.exit_code }
        };

        FILESYSTEM.Write(state_file, json.dump());
    } catch (const std::exception& e) {
        VLOG(1)
            << "Failed to write state file "
            << state_file
            << " because: "
            << e.what();
    }

    return true;
}

bool winss::SuperviseStateFile::Read(winss::SuperviseState* state) const {
    if (state == nullptr) {
        return false;
    }

    try {
        std::string content = FILESYSTEM.Read(state_file);
        if (content.empty()) {
            return false;
        }

        auto ss = std::stringstream(content);
        auto json = nlohmann::json::parse(ss);

        for (auto it = json.begin(); it != json.end(); ++it) {
            std::string key = it.key();
            auto value = it.value();

            if (key == "time" && value.is_string()) {
                state->time = winss::Utils::ConvertFromISOString(value);
            } else if (key == "last" && value.is_string()) {
                state->last = winss::Utils::ConvertFromISOString(value);
            } else if (key == "proc" && value.is_string() &&
                value == std::string("run")) {
                state->is_run_process = true;
            } else if (key == "state" && value.is_string() &&
                value == std::string("up")) {
                state->is_up = true;
            } else if (key == "initial" && value.is_string() &&
                value == std::string("up")) {
                state->initially_up = true;
            } else if (key == "count" && value.is_number()) {
                state->up_count = value;
            } else if (key == "remaining" && value.is_number()) {
                state->remaining_count = value;
            } else if (key == "pid" && value.is_number()) {
                state->pid = value;
            } else if (key == "exit" && value.is_number()) {
                state->exit_code = value;
            }
        }
    } catch (const std::exception& e) {
        VLOG(1)
            << "Failed to read state file "
            << state_file
            << " because: "
            << e.what();
        return false;
    }

    return true;
}

std::string winss::SuperviseStateFile::Format(
    const winss::SuperviseState& state, bool is_up) const {
    std::stringstream ss;

    bool is_run = state.is_up && state.is_run_process;

    if (is_run) {
        ss << "up (pid " << state.pid << ")";
    } else {
        ss << "down";

        if (state.exit_code != 0) {
            ss << " (exit code " << state.exit_code << ")";
        }
    }

    auto now = std::chrono::system_clock::now();
    auto delay = std::chrono::duration_cast<std::chrono::seconds>(
        now - state.last).count();

    if (delay >= 0) {
        ss << " " << delay << " seconds";
    }

    if (is_up) {
        if (state.up_count > 1) {
            ss << ", started " << state.up_count << " times";
        }

        if (is_run) {
            if (!state.initially_up) {
                ss << ", normally down";
            }

            if (state.remaining_count == 0) {
                ss << ", want down";
            }
        } else {
            if (state.initially_up) {
                ss << ", normally up";
            }

            if (state.remaining_count != 0) {
                ss << ", want up";
            } else if (state.up_count > 0) {
                ss << ", paused";
            }
        }
    }

    return ss.str();
}
