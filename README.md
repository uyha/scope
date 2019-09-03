# scope17
C++17 version of `unique_resource` and scope_guards from LFTS3

List of tested compilers:
- Linux and gcc 8.3
- Windows and msvc 2017
- Windows and msvc 2019

Status of tests in CI servers:

| Branch | Travis | Appveyor |
| ---- | -------- | -------- |
| **Master** | [![Travis Build](https://travis-ci.org/menuet/scope17.svg?branch=master)](https://travis-ci.org/menuet/scope17) | [![Appveyor Build](https://ci.appveyor.com/api/projects/status/g4oox7x8f7ytfmv6/branch/master?svg=true)](https://ci.appveyor.com/project/menuet/scope17/branch/master) |

How to build and run the tests locally:
```
mkdir build && cd build && cmake -DCMAKE_VERBOSE_MAKEFILE=TRUE .. && cmake --build . --config Debug && ctest -C Debug
```
