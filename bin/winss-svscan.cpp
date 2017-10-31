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
#include "winss/winss.hpp"
#include "optionparser/optionparser.hpp"
#include "easylogging/easylogging++.hpp"
#include "winss/filesystem_interface.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/svscan/svscan.hpp"
#include "winss/svscan/controller.hpp"
#include "winss/pipe_server.hpp"
#include "winss/pipe_name.hpp"
#include "winss/ctrl_handler.hpp"
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

struct Settings {
    fs::path scan_dir;
    DWORD rescan = INFINITE;
    bool signals = false;
    int verbose_level = 0;
};

struct Arg : public option::Arg {
    static option::ArgStatus Required(const option::Option& option, bool msg) {
        if (option.arg != 0)
            return option::ARG_OK;

        if (msg) {
            std::cerr
                << "Option '"
                << option.name
                << "' requires an argument\n";
        }

        return option::ARG_ILLEGAL;
    }
};

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE, TIMEOUT, SIGNALS };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", Arg::None,
        "Usage: winss-svscan" SUFFIX ".exe [options] [scandir]\n\n"
        "Options:"
    },
    {
        HELP, 0, "h", "help", Arg::None,
        "  --help  \tPrint usage and exit."
    },
    {
        VERSION, 0, "", "version", Arg::None,
        "  --version  \tPrint the current version of winss and exit."
    },
    {
        VERBOSE, 0, "v", "verbose", Arg::Optional,
        "  -v[<level>], \t--verbose[=<level>]  \tSets the verbose level."
    },
    {
        TIMEOUT, 0, "t", "timeout", Arg::Required,
        "  -t<rescan>, \t--timeout=<rescan>  \tSets the rescan timeout."
    },
    {
        SIGNALS, 0, "s", "signals", Arg::None,
        "  -s, \t--signals  \tDivert signals."
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

    if (options[HELP]) {
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

    Settings settings{};
    if (parse.nonOptionsCount() < 1) {
        settings.scan_dir = fs::current_path();
    } else {
        settings.scan_dir = parse.nonOption(0);
    }

    settings.scan_dir = FILESYSTEM.Absolute(settings.scan_dir);

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
        case TIMEOUT:
            try {
                settings.rescan = std::strtoul(opt.arg, nullptr, 10);
            } catch (const std::exception&) {
                std::cerr
                    << "Option "
                    << opt.name
                    << " requires a numeric argument";
            }
            break;
        case SIGNALS:
            settings.signals = true;
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

int main(int argc, char* argv[]) {
    winss::AttachCtrlHandler();

    Settings settings = ParseArgs(argc, argv);
    ConfigureLogger(settings);

    winss::WaitMultiplexer multiplexer;

    winss::PipeName pipe_name(settings.scan_dir, winss::SvScan::kMutexName);
    winss::InboundPipeServer inbound({
        pipe_name.Append("control"),
        winss::NotOwned(&multiplexer)
    });

    winss::SvScan svscan(winss::NotOwned(&multiplexer), settings.scan_dir,
        settings.rescan, settings.signals, winss::GetCloseEvent());
    winss::SvScanController controller(winss::NotOwned(&svscan),
        winss::NotOwned(&inbound));
    return multiplexer.Start();
}
