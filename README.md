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

## Usage

This is a header-only so you can download [scope.hpp](https://raw.githubusercontent.com/uyha/scope/main/include/scope.hpp)
to your project and start using it.

## Features

This library provides the following 4 classes:

- `scope_exit`
- `scope_fail`
- `scope_success`
- `unique_resource`

a utility function `make_unique_resource_checked`, and 3 macros:

- `SCOPE_EXIT`
- `SCOPE_FAIL`
- `SCOPE_SUCCESS`

### `scope_exit`, `scope_fail`, `scope_success`, and `SCOPE_*` macros

These classes provide a way to run code when a scope ends (either by reaching the of a
block or by exception) without creating RAII classes. These classes differ in when they
invoke their functions.

| Invoke when scope ends | with exception | without exception |
| ---------------------- | -------------- | ----------------- |
| `scope_exit`           | yes            | yes               |
| `scope_fail`           | yes            | no                |
| `scope_success`        | no             | yes               |

The macros provide the same functionality to their lower-case counterparts, but they
also create a unique variable when they are called. This frees the user from having to
create the variable with unique names manually.

> **Warning**:
> These `SCOPE_*` macros rely on the `__COUNTER__` when it's available, which
> should not have any problem when the `SCOPE_*` macros are used on the same line.
> However, when the `__COUNTER__` macro is not available, `__LINE__` is used
> instead. This leads to compilation error when using the `SCOPE_*` macros on the
> same line. So either use compilers that support the `__COUNTER__` macros (GCC,
> Clang, and MSVC all support it), or make sure that the `SCOPE_*` are not use on the
> same line.

#### Example

```cpp
#include <iostream>

#include <scope.hpp>

int main() {
  try {
    auto on_exit    = scope::scope_exit([]{ std::cout << "exit\n"; });
    auto on_fail    = scope::scope_fail([]{ std::cout << "fail\n"; });
    auto on_success = scope::scope_success([]{ std::cout << "success\n"; });

    SCOPE_EXIT([]{ std::cout << "scope_exit\n"; });
    SCOPE_FAIL([]{ std::cout << "scope_fail\n"; });
    SCOPE_SUCCESS([]{ std::cout << "scope_success\n"; });
    throw 42;
  } catch (...) {
  }
}
```

The above snippet prints

```
scope_fail
scope_exit
fail
exit
```

### `unique_resource` and `make_unique_resource_checked`

`unique_resource` holds an object and runs a function when the it goes out of scope
while being "active". A `unique_resource` object is not "active" when its `release`
function is called, or when it's created with the `make_unique_resource_checked`
function and the resource is not valid (check example to see what valid means).

```cpp
#include <cstdio>
#include <iostream>

#include <scope.hpp>

int main() {
  auto closer = [](FILE *handle) {
    static auto count = 0;
    std::cout << "called: " << count++ << std::endl;
    std::fclose(handle);
  };
  {
    auto filename = "hello.txt";
    auto file = scope::make_unique_resource_checked(std::fopen(filename, "w"),
                                                    nullptr, closer);
  }
  {
    auto filename = "/doesnotexist/hello.txt";
    auto file = scope::make_unique_resource_checked(std::fopen(filename, "w"),
                                                    nullptr, closer);
  }
}
```

The above snippet prints

```
called: 0
```

The second block will not invoke `closer` because the value returned from
`std::fopen(file, "w")` is `nullptr`, hence it's consider not "valid" and the deleter is
not invoked.
