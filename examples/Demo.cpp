#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../scope.hpp"

namespace {
using namespace std::experimental;

using std::filesystem::path;

void copy_file_transact(path const &from, path const &to) {
    path t{to};
    t += path{".deleteme"};
    auto guard = scope_fail{[t] { remove(t); }};
    copy_file(from, t);
    rename(t, to);
}

void DemonstrateTransactionFilecopy() {
    std::string name("hello.txt");
    path to("/tmp/scope_hello.txt");
    {
        std::ofstream ofs{name};
        ofs << "Hello world\n";
    }

    copy_file_transact(name, to);
    auto guard = scope_success{[to] { remove(to); }};
    assert(exists(to) == true);
}

void demonstrate_unique_resource_with_stdio() {
    const std::string filename = "hello.txt";
    auto fclose = [](auto file) {
        ::fclose(file);
    };  // not allowed to take address const std::string filename = "hello.txt";
    {
        auto file = make_unique_resource_checked(::fopen(filename.c_str(), "w"),
                                                 nullptr, fclose);
        ::fputs("Hello World!\n", file.get());
        assert(file.get() != nullptr);
    }
    {
        std::ifstream input{filename};
        std::string line{};
        getline(input, line);
        assert("Hello World!" == line);
        getline(input, line);
        assert(input.eof());
    }
    ::unlink(filename.c_str());
    {
        auto file = make_unique_resource_checked(
            ::fopen("nonexistingfile.txt", "r"), nullptr, fclose);
        assert(nullptr == file.get());
    }
}

void demontrate_unique_resource_with_POSIX_IO() {
    const std::string filename = "hello1.txt";
    auto close = [](auto fd) { ::close(fd); };
    {
        auto file = make_unique_resource_checked(
            ::open(filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666), -1,
            close);
        ::write(file.get(), "Hello World!\n", 12u);
        assert(file.get() != -1);
    }
    {
        std::ifstream input{filename};
        std::string line{};
        getline(input, line);
        assert("Hello World!" == line);
        getline(input, line);
        assert(input.eof());
    }
    ::unlink(filename.c_str());
    {
        auto file = make_unique_resource_checked(
            ::open("nonexistingfile.txt", O_RDONLY), -1, close);
        assert(-1 == file.get());
    }
}

void demo_scope_exit_fail_success() {
    std::ostringstream out{};
    auto lam = [&] { out << "called "; };
    try {
        auto v = scope_exit([&] { out << "always "; });
        auto w = scope_success([&] { out << "not "; });
        auto x = scope_fail(lam);  // called too
        throw 42;
    } catch (...) {
        auto y = scope_fail([&] { out << "not "; });
        auto z = scope_success([&] { out << "handled"; });  // called too
    }
    assert("called always handled" == out.str());
}
}  // namespace

int main() {
    DemonstrateTransactionFilecopy();
    demonstrate_unique_resource_with_stdio();
    demontrate_unique_resource_with_POSIX_IO();
    demo_scope_exit_fail_success();
}
