#include <winss/winss.hpp>
#include <optionparser/optionparser.hpp>
#include <easylogging/easylogging++.hpp>
#include <winss/ctrl_handler.hpp>
#include <winss/filesystem_interface.hpp>
#include <winss/not_owning_ptr.hpp>
#include <winss/wait_multiplexer.hpp>
#include <winss/path_mutex.hpp>
#include <winss/pipe_client.hpp>
#include <winss/svscan/svscan.hpp>
#include <winss/svscan/controller.hpp>
#include <winss/control.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

struct Settings {
    fs::path scan_dir;
    std::vector<char> commands;
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

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE, ALARM, ABORT, NUKE, QUIT };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", Arg::None,
        "Usage: winss-svscanctl" SUFFIX ".exe [options] scandir\n\n"
        "Options:"
    },
    {
        HELP, 0, "h", "help", Arg::None,
        "  --help  \tPrint usage and exit."
    },
    {
        VERSION, 0, "", "version", option::Arg::None,
        "  --version  \tPrint the current version of winss and exit."
    },
    {
        VERBOSE, 0, "v", "verbose", Arg::Optional,
        "  -v[<level>], \t--verbose[=<level>]  \tSets the verbose level."
    },
    {
        ALARM, 0, "a", "alarm", Arg::None,
        "  -a, \t--alarm  \tPerform a scan of scandir."
    },
    {
        ABORT, 0, "b", "abort", Arg::None,
        "  -b, \t--abort  \tClose svscan only."
    },
    {
        NUKE, 0, "n", "nuke", Arg::None,
        "  -n, \t--nuke  \tPrune supervision tree."
    },
    {
        QUIT, 0, "q", "quit", Arg::None,
        "  -q, \t--quit  \tStop supervised process and svscan."
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
        std::cerr << "Error: scandir is required!" << std::endl;
        std::exit(100);
    }

    Settings settings{};
    settings.scan_dir = FILESYSTEM.Absolute(parse.nonOption(0));

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
        case ALARM:
            settings.commands.push_back(winss::SvScanController::kAlarm);
            break;
        case ABORT:
            settings.commands.push_back(winss::SvScanController::kAbort);
            break;
        case NUKE:
            settings.commands.push_back(winss::SvScanController::kNuke);
            break;
        case QUIT:
            settings.commands.push_back(winss::SvScanController::kQuit);
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

    if (settings.commands.empty()) {
        return 0;
    }

    winss::PathMutex mutex(settings.scan_dir, winss::SvScan::kMutexName);
    if (mutex.CanLock()) {
        return 100;
    }

    winss::WaitMultiplexer multiplexer;
    multiplexer.AddCloseEvent(winss::GetCloseEvent(), 0);

    winss::PipeName pipe_name(settings.scan_dir, winss::SvScan::kMutexName);
    winss::OutboundPipeClient outbound({
        pipe_name.Append("control"),
        winss::NotOwned(&multiplexer)
    });

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE);
    winss::OutboundControlItem svc_control(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&outbound),
        settings.commands, "svscanctl");

    return control.Start();
}
