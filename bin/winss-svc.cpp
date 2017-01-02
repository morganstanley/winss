#include <filesystem>
#include <iostream>
#include <vector>
#include "winss/winss.hpp"
#include "optionparser/optionparser.hpp"
#include "easylogging/easylogging++.hpp"
#include "winss/ctrl_handler.hpp"
#include "winss/filesystem_interface.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/path_mutex.hpp"
#include "winss/pipe_client.hpp"
#include "winss/supervise/supervise.hpp"
#include "winss/supervise/controller.hpp"
#include "winss/supervise/state_listener.hpp"
#include "winss/control.hpp"
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

struct Settings {
    fs::path service_dir;
    std::vector<char> commands;
    winss::SuperviseStateListenerAction wait =
        winss::SuperviseStateListenerAction::NO_WAIT;
    DWORD timeout = INFINITE;
    int verbose_level = 0;
};

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE, KILL, TERM,
    ONCE, DOWN, UP, EXIT, ONCEATMOST, TIMEOUT, WAIT };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", option::Arg::None,
        "Usage: winss-svc" SUFFIX ".exe [options] servicedir\n\n"
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
        KILL, 0, "k", "kill", option::Arg::None,
        "  -k, \t--kill  \tTerminate the process."
    },
    {
        TERM, 0, "t", "term", option::Arg::None,
        "  -t, \t--term  \tSend a CTRL+BREAK to the process"
    },
    {
        ONCE, 0, "o", "once", option::Arg::None,
        "  -o, \t--once  \tEquivalent to '-uO'."
    },
    {
        DOWN, 0, "d", "down", option::Arg::None,
        "  -d, \t--down  \tStop the supervised process."
    },
    {
        UP, 0, "u", "up", option::Arg::None,
        "  -u, \t--up  \tStarts the supervised process."
    },
    {
        EXIT, 0, "x", "exit", option::Arg::None,
        "  -x, \t--exit  \tStop the process and supervisor."
    },
    {
        ONCEATMOST, 0, "O", "onceatmost", option::Arg::None,
        "  -O, \t--onceatmost  \tOnly run supervised process once."
    },
    {
        TIMEOUT, 0, "t", "timeout", option::Arg::Optional,
        "  -t<ms>, \t--timeout=<ms>"
        "  \tWait timeout in milliseconds if -w is specified."
    },
    {
        WAIT, 0, "w", "wait", option::Arg::Optional,
        "  -w<dDur>, \t--wait=<dDur>"
        "  \tWait on (d)own/finishe(D)/(u)p/(r)estart."
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
        std::cerr << "Error: servicedir is required!" << std::endl;
        std::exit(100);
    }

    Settings settings{};
    settings.service_dir = FILESYSTEM.Absolute(parse.nonOption(0));

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
        case KILL:
            settings.commands.push_back(winss::SuperviseController::kSvcKill);
            break;
        case TERM:
            settings.commands.push_back(winss::SuperviseController::kSvcTerm);
            break;
        case ONCE:
            settings.commands.push_back(winss::SuperviseController::kSvcOnce);
            break;
        case DOWN:
            settings.commands.push_back(winss::SuperviseController::kSvcDown);
            break;
        case UP:
            settings.commands.push_back(winss::SuperviseController::kSvcUp);
            break;
        case EXIT:
            settings.commands.push_back(winss::SuperviseController::kSvcExit);
            break;
        case ONCEATMOST:
            settings.commands.push_back(
                winss::SuperviseController::kSvcOnceAtMost);
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
        case WAIT:
            if (opt.arg != nullptr) {
                switch (opt.arg[0]) {
                case 'd':
                    settings.wait =
                        winss::SuperviseStateListenerAction::WAIT_DOWN;
                    break;
                case 'D':
                    settings.wait =
                        winss::SuperviseStateListenerAction::WAIT_FINISHED;
                    break;
                case 'u':
                    settings.wait =
                        winss::SuperviseStateListenerAction::WAIT_UP;
                    break;
                case 'r':
                    settings.wait =
                        winss::SuperviseStateListenerAction::WAIT_RESTART;
                    break;
                default:
                    std::cerr
                        << "Option "
                        << opt.name
                        << " requires either d/D/u/r";
                    break;
                }
            }
            break;
        }
    }

    if (settings.commands.empty()) {
        option::printUsage(std::cout, usage);
        std::exit(100);
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

    winss::PathMutex mutex(settings.service_dir, winss::Supervise::kMutexName);
    if (mutex.CanLock()) {
        return 100;
    }

    winss::WaitMultiplexer multiplexer;
    multiplexer.AddCloseEvent(winss::GetCloseEvent(), 0);

    winss::PipeName pipe_name(settings.service_dir,
        winss::Supervise::kMutexName);
    winss::InboundPipeClient inbound({
        pipe_name.Append("event"),
        winss::NotOwned(&multiplexer)
    });
    winss::OutboundPipeClient outbound({
        pipe_name.Append("control"),
        winss::NotOwned(&multiplexer)
    });

    winss::Control control(winss::NotOwned(&multiplexer), settings.timeout);

    winss::SuperviseStateFile state_file(settings.service_dir);
    winss::SuperviseStateListener state_listener(state_file, settings.wait);
    winss::InboundControlItem inbound_event(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&inbound),
        winss::NotOwned(&state_listener), "svc");

    winss::OutboundControlItem outbound_control(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&outbound),
        settings.commands, "svc");

    return control.Start();
}
