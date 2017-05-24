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

#include <filesystem>
#include <iostream>
#include <vector>
#include <memory>
#include "winss/winss.hpp"
#include "optionparser/optionparser.hpp"
#include "easylogging/easylogging++.hpp"
#include "winss/ctrl_handler.hpp"
#include "winss/filesystem_interface.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/path_mutex.hpp"
#include "winss/pipe_client.hpp"
#include "winss/sha256.hpp"
#include "winss/supervise/supervise.hpp"
#include "winss/supervise/controller.hpp"
#include "winss/supervise/state_listener.hpp"
#include "winss/control.hpp"
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

struct Settings {
    std::vector<fs::path> service_dirs;
    winss::SuperviseStateListenerAction wait =
        winss::SuperviseStateListenerAction::NO_WAIT;
    bool wait_all = true;
    DWORD timeout = INFINITE;
    int verbose_level = 0;
};

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE, UP, DOWN, FINISHED,
    OR, AND, TIMEOUT };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", option::Arg::None,
        "Usage: winss-svwait" SUFFIX ".exe [options] servicedir\n\n"
        "Options:"
    },
    {
        HELP, 0, "h", "help", option::Arg::None,
        "  --help  \tPrint usage and exit."
    },
    {
        VERSION, 0, "", "version", option::Arg::None,
        "  --version  \tPrint the current version of winss and exit."
    },
    {
        VERBOSE, 0, "v", "verbose", option::Arg::Optional,
        "  -v[<level>], \t--verbose[=<level>]  \tSets the verbose level."
    },
    {
        UP, 0, "u", "up", option::Arg::None,
        "  -u, \t--up  \tWait until the services are up."
    },
    {
        DOWN, 0, "d", "down", option::Arg::None,
        "  -d, \t--down  \tWait until the services are down."
    },
    {
        FINISHED, 0, "D", "finshed", option::Arg::None,
        "  -D, \t--finished  \tWait until the services are really down."
    },
    {
        OR, 0, "o", "or", option::Arg::None,
        "  -o, \t--or  \tWait until one of the services comes up or down."
    },
    {
        AND, 0, "a", "and", option::Arg::None,
        "  -a, \t--and  \tWait until all of the services comes up or down."
    },
    {
        TIMEOUT, 0, "t", "timeout", option::Arg::Optional,
        "  -t<ms>, \t--timeout=<ms>  \tWait timeout in milliseconds."
    },
    { 0, 0, 0, 0, 0, 0 }
};

Settings ParseArgs(int argc, char* argv[]) {
    /* Skip program name argv[0] if present */
    argc -= (argc > 0);
    argv += (argc > 0);

    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error()) {
        std::exit(100);
    }

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        std::exit(100);
    }

    if (options[VERSION]) {
        std::cout
            << "winss "
            << VERSION_MAJOR << "."
            << VERSION_MINOR << "."
            << VERSION_REVISION << "."
            << VERSION_BUILD;
#ifdef GIT_COMMIT_SHORT
        std::cout << "-" << GIT_COMMIT_SHORT;
#endif
        std::cout << std::endl;
        std::exit(0);
    }

    if (parse.nonOptionsCount() < 1) {
        std::cerr
            << "Error: at least one servicedir is required!"
            << std::endl;
        std::exit(100);
    }

    Settings settings{};
    for (int i = 0; i < parse.nonOptionsCount(); i++) {
        std::string name(parse.nonOption(i));

        if (name.empty() || name.front() == L'.'
            || !FILESYSTEM.DirectoryExists(name)) {
            VLOG(4) << "Skipping directory " << name;
        } else {
            settings.service_dirs.push_back(
                std::move(FILESYSTEM.Absolute(name)));
        }
    }

    for (int i = 0; i < parse.optionsCount(); ++i) {
        option::Option& opt = buffer[i];

        switch (opt.index()) {
        case VERBOSE:
            if (opt.arg == nullptr) {
                settings.verbose_level = el::base::consts::kMaxVerboseLevel;
            } else {
                try {
                    settings.verbose_level = std::strtol(opt.arg, nullptr, 10);
                } catch (const std::exception&) {
                    std::cerr
                        << "Option "
                        << opt.name
                        << " requires a numeric argument";
                }
            }
            break;
        case UP:
            settings.wait = winss::SuperviseStateListenerAction::WAIT_UP;
            break;
        case DOWN:
            settings.wait = winss::SuperviseStateListenerAction::WAIT_DOWN;
            break;
        case FINISHED:
            settings.wait = winss::SuperviseStateListenerAction::WAIT_FINISHED;
            break;
        case OR:
            settings.wait_all = false;
            break;
        case AND:
            settings.wait_all = true;
            break;
        case TIMEOUT:
            if (opt.arg != nullptr) {
                try {
                    settings.timeout = std::strtoul(opt.arg, nullptr, 10);

                    if (settings.timeout == 0) {
                        settings.timeout = INFINITE;
                    }
                } catch (const std::exception&) {
                    std::cerr
                        << "Option "
                        << opt.name
                        << " requires a numeric argument";
                    settings.timeout = INFINITE;
                }
            }
            break;
        }
    }

    return settings;
}

void ConfigureLogger(const Settings& settings) {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    el::Loggers::reconfigureAllLoggers(defaultConf);

    if (settings.verbose_level > 0) {
        el::Loggers::setVerboseLevel(settings.verbose_level);
    }
}

struct WaitItem {
    fs::path service_dir;
    std::unique_ptr<winss::InboundPipeClient> inbound_pipe;
    std::unique_ptr<winss::SuperviseStateFile> state_file;
    std::unique_ptr<winss::SuperviseStateListener> state_listener;
    std::unique_ptr<winss::InboundControlItem> inbound_event;
};

int main(int argc, char* argv[]) {
    winss::AttachCtrlHandler();

    Settings settings = ParseArgs(argc, argv);
    ConfigureLogger(settings);

    winss::WaitMultiplexer multiplexer;
    multiplexer.AddCloseEvent(winss::GetCloseEvent(), 0);

    winss::Control control(winss::NotOwned(&multiplexer),
        settings.timeout, settings.wait_all);

    std::vector<WaitItem> wait_items;

    for (const fs::path& service_dir : settings.service_dirs) {
        winss::PathMutex mutex(service_dir, winss::Supervise::kMutexName);
        if (!mutex.CanLock()) {
            WaitItem wait_item{ service_dir };

            winss::PipeName pipe_name(service_dir,
                winss::Supervise::kMutexName);

            wait_item.inbound_pipe =
                std::make_unique<winss::InboundPipeClient>(
                winss::PipeClientConfig{
                pipe_name.Append("event"),
                winss::NotOwned(&multiplexer)
            });

            wait_item.state_file = std::make_unique<winss::SuperviseStateFile>(
                service_dir);

            wait_item.state_listener =
                std::make_unique<winss::SuperviseStateListener>(
                    *wait_item.state_file, settings.wait);

            wait_item.inbound_event =
                std::make_unique<winss::InboundControlItem>(
                winss::NotOwned(&multiplexer), winss::NotOwned(&control),
                winss::NotOwned(wait_item.inbound_pipe.get()),
                winss::NotOwned(wait_item.state_listener.get()),
                winss::SHA256::CalculateDigest(service_dir.string()));

            wait_items.push_back(std::move(wait_item));
        }
    }

    if (wait_items.empty()) {
        VLOG(1) << "There were no running service directories specified";
        return 0;
    }

    int return_code = control.Start();
    if (0 == return_code && !settings.wait_all) {
        for (const WaitItem& wait_item : wait_items) {
            if (wait_item.inbound_event->Completed()) {
                std::cout << wait_item.service_dir << std::endl;
                break;
            }
        }
    }

    return return_code;
}
