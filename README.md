# scope

[![Build and test](https://github.com/uyha/scope/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/uyha/scope/actions/workflows/build-and-test.yml)

A standalone C++17 header-only library for Andrei Alexandrescu's [Declarative Control Flow](https://www.youtube.com/watch?v=WjTrfoiB0MQ)
and [P0052](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0052r10.pdf).

This project is a fork from Peter Sommerlad's [scope17](https://github.com/PeterSommerlad/scope17)
implementation with changes to make it appropriate for being a standalone
library.

## Build and test

```sh
cmake -B build -S .
cmake --build build
ctest --test-dir build/tests
```
