# Windows Supervision Suite (*winss*)

[![Build status](https://ci.appveyor.com/api/projects/status/3f2rxoh1sqfro4gk?svg=true)](https://ci.appveyor.com/project/keithhendry/winss-f91wg)
[![codecov](https://codecov.io/gh/Morgan-Stanley/winss/branch/master/graph/badge.svg)](https://codecov.io/gh/Morgan-Stanley/winss)
[![Documentation Status](https://readthedocs.org/projects/winss/badge/?version=latest)](http://winss.readthedocs.io/en/latest/?badge=latest)
[![GitHub release](https://img.shields.io/github/release/Morgan-Stanley/winss.svg)](https://github.com/Morgan-Stanley/winss/releases/latest)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

*winss* is a small suite of tools for Windows that is designed to be as close to
its counterpart [S6](http://skarnet.org/software/s6/) as possible using native
Windows features. It's core function is to allow process supervision but it
provides tools to supervise a collection of processes, handle logging, and
also administration.

Using this framework it is simple to handle complex scenarios with little code
such as  adding/removing services and starting/stopping services in a particular
sequence. There is no requirement to change an application to handle events
like traditional Windows services. *winss* can handle simple console applications
the way it should have been.

## Documentation

You will find documentation for using winss on the [Read the Docs](http://winss.readthedocs.io/en/latest/) site.

## Getting Started for Developers

These instructions will get you a copy of *winss* up and running on your local
machine for development and testing purposes. See deployment for notes on
how to deploy *winss* on a live system.

### Prerequisites

The following are required to build and test *winss*.

* [Visual Studio 2015 (Update 3)](https://www.visualstudio.com/downloads/)
* [git](https://git-scm.com/download/win)
* [Premake 5](https://premake.github.io/download.html)
* [Python 2.7](https://www.python.org/downloads/)
* [Chocolatey](https://chocolatey.org/install)

### Installing

A step by step guide for getting *winss* development environment running.

1. [Fork](https://help.github.com/articles/fork-a-repo/) the
   [winss](https://github.com/Morgan-Stanley/winss) repo.
2. Clone the forked copy using `git clone https://github.com/username/winss.git`
3. Run `premake5 vs2015` in the src folder.
4. Open The `WindowsSupervisionSuite.sln` in Visual Studio.
5. Build *Debug* or *Release*.

#### Optional

Using an elevated command prompt:

1. `choco install opencppcoverage`
2. `pip install cpplint`

## Running the tests

In the `build\bin\x64\Debug` directory there will be a `winss-test.exe`.
Details on the command line options can be found [here](https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#running-test-programs-advanced-options).

### With code coverage

To run with code coverage then use `OpenCppCoverage.exe` installed as part of
the optional prerequisites.

```
OpenCppCoverage.exe --modules=winss --sources=lib\winss\* --export_type=cobertura:build\coverage.xml -q -- build\bin\x64\Debug\winss-test.exe --gtest_output=xml:build\testresults.xml
```

There is a powershell script that can automate this for you:

```
powershell .\tools\Run-Tests.ps1
```

### Check code style

*winss* follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
To check then use `cpplint`

```
cpplint --extension=hpp,cpp --headers=hpp --filter=-build/c++11 <filename>
```

Or use the powershell script with an optional filename:

```
powershell  .\tools\Run-Lint.ps1 [FILENAME]
```

## Deployment

These instructions will get you a copy of *winss* up and running on your local
machine using a production version.

### Prerequisites

1. [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145)
   needs to be installed.

### Installing

Download the [latest Release](https://github.com/Morgan-Stanley/winss/releases/latest)
build to your local system and add the directory to the %PATH%.

## Built With

* [Easylogging++](https://github.com/easylogging/easyloggingpp) - for logging.
* [The Lean Mean C++ Option Parser](http://optionparser.sourceforge.net/) -
  for parsing command line options.
* [JSON for Modern C++](https://github.com/nlohmann/json) - for reading/writing
  JSON.
* [HowardHinnant/date](https://github.com/HowardHinnant/date) - for
  converting to and from ISO 8601 strings with millisecond precision.
* [Google Test](https://github.com/google/googletest) - for testing/mocking.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of
conduct, and the process for submitting pull requests to us.

## License

This project is licensed under the Apache License - see the
[LICENSE.md](LICENSE.md) file for details
