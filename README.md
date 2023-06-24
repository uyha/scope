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

## Features

This library provides the following 4 classes:

- `scope_exit`
- `scope_fail`
- `scope_success`
- `unique_resource`

and a utility function `make_unique_resource_checked`

### `scope_exit`, `scope_fail`, and `scope_success`

These classes provide a way to run code when a scope ends (either by reaching the of a
block or by exception) without creating RAII classes. These classes differ in when they
invoke their functions.

| Invoke when scope ends | with exception | without exception |
| ---------------------- | -------------- | ----------------- |
| `scope_exit`           | yes            | yes               |
| `scope_fail`           | yes            | no                |
| `scope_success`        | no             | yes               |

#### Example

```cpp
#include <iostream>

#include <scope.hpp>

int main() {
  try {
    auto on_exit    = scope::scope_exit([]{ std::cout << "exit" << std::endl; });
    auto on_fail    = scope::scope_fail([]{ std::cout << "fail" << std::endl; });
    auto on_success = scope::scope_success([]{ std::cout << "success" << std::endl; });
    throw 42;
  } catch (...) {
  }
}
```

The above snippet prints

```
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
