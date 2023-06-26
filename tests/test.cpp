/*
 * MIT License

Copyright (c) 2016/2017 Peter Sommerlad

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include "scope.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#define open _open
#define close _close
#define unlink _unlink
#define write _write
#endif

#include <filesystem>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

// #define CHECK_COMPILE_ERRORS

using scope::make_unique_resource_checked;
using scope::scope_exit;
using scope::scope_fail;
using scope::scope_success;
using scope::unique_resource;

TEST_CASE("DemoFstream") {
  {
    std::ofstream ofs{"demo_hello.txt"};
    ofs << "Hello world\n";
  }
  {
    std::ifstream ifs{"demo_hello.txt"};
    std::string s{};
    REQUIRE(getline(ifs, s));
    REQUIRE("Hello world" == s);
  }
}

TEST_CASE("Copy file transaction") {}
using std::filesystem::path;
void copy_file_transact(path const &from, path const &to) {
  path t{to};
  t += path{".deleteme"};
  auto guard = scope_fail{[t] { remove(t); }};
  copy_file(from, t);
  rename(t, to);
}
void DemonstrateTransactionFilecopy() {}

TEST_CASE("Demonstrate surprising returned fromv behavior") {
  size_t len{0xffffffff};
  auto const s = [&len] {
    std::string s{"a long string to prevent SSO, so let us see if the move and RVO inhibit really is destructive"};
    scope_exit guard{[&] { len = s.size(); }};
    return std::move(s); // pessimize copy elision to demonstrate effect
  }();
  // exected:    REQUIRE(s.size() == len);
  // what really happens
  REQUIRE(0 == len);
}

TEST_CASE("Insane Bool") {
  std::ostringstream out{};
  {
    auto guard = scope_exit([&] { out << "done\n"; });
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test scope_exit with C++17 deducing ctors") {
  std::ostringstream out{};
  {
    scope_exit guard([&] { out << "done\n"; });
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test scope_fail with C++17 deducing ctors") {
  std::ostringstream out{};
  {
    scope_fail guard([&] { out << "not done\n"; });
  }
  REQUIRE("" == out.str());
  try {
    scope_fail guard([&] { out << "done\n"; });
    throw 0;
  } catch (const int) {
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test scope_success with C++17 deducing ctors") {
  std::ostringstream out{};
  {
    scope_success guard([&] { out << "done\n"; });
  }
  REQUIRE("done\n" == out.str());
  try {
    scope_success guard([&] { out << "not done\n"; });
    throw 0;
  } catch (int) {
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test dimissed guard") {
  std::ostringstream out{};
  {
    auto guard         = scope_exit([&] { out << "done1\n"; });
    auto guard2dismiss = scope_exit([&] { out << "done2\n"; });
    guard2dismiss.release();
  }
  REQUIRE("done1\n" == out.str());
}

TEST_CASE("Test throw doesn't crash it") { // LEWG wants it to crash!
  std::ostringstream out{};
  {
    auto guard  = scope_exit([&] { out << "done\n"; });
    auto guard1 = scope_exit([] { throw 42; });
    guard1.release(); // we no longer want throwing scope guards
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test scope_exit with reference_wrapper") {
  std::ostringstream out{};
  const auto &lambda = [&] { out << "lambda done.\n"; };
  { auto guard = scope_exit(std::cref(lambda)); }
  REQUIRE("lambda done.\n" == out.str());
}
struct non_assignable_resource {
  non_assignable_resource() = default;
  non_assignable_resource(int) {}
  void operator=(const non_assignable_resource &) = delete;
  non_assignable_resource &operator=(non_assignable_resource &&) noexcept(false) {
    throw "buh";
  };
  non_assignable_resource(non_assignable_resource &&) = default;
};

TEST_CASE("Test scope_exit with non assignable resource and reset") {
  std::ostringstream out{};
  const auto &lambda = [&](auto &&) { out << "lambda done.\n"; };
  {
    auto guard = unique_resource(non_assignable_resource{}, std::cref(lambda));
    // guard.reset(2);//throws... need to figure out, what I wanted to trigger here. AH, compile error?
  }
  REQUIRE("lambda done.\n" == out.str());
}

// by Eric Niebler, adapted for unit testing
struct throwing_copy {
  throwing_copy(const char *sz, std::ostream &os)
      : sz_{sz}
      , out{os} {}
  throwing_copy(const throwing_copy &other)
      : out{other.out} {
    using namespace std::literals;
    throw "throwing_copy copy attempt:"s + sz_;
  }
  void operator()() const {
    out << sz_ << std::endl;
  }
  template <typename RES>
  void operator()(RES const &res) const {
    out << sz_ << res << std::endl;
  }

  bool operator==(int i) const & {
    return i == 0;
  }
  friend std::ostream &operator<<(std::ostream &os, throwing_copy const &t) {
    return os << t.sz_;
  }

private:
  const char *sz_{""};
  std::ostream &out;
};

TEST_CASE("Tests from Eric Niebler scope_exit with throwing function object") {
  std::ostringstream out{};
  throwing_copy c{"called anyway", out};
  REQUIRE_THROWS(scope_exit(c));
  REQUIRE("called anyway\n" == out.str());
}

TEST_CASE("Tests from Eric Niebler scope_success with throwing function object") {
  std::ostringstream out{};
  throwing_copy c{"Oh noes!!!", out};
  REQUIRE_THROWS(scope_success(c));
  REQUIRE("" == out.str());
}

TEST_CASE("Tests from Eric Niebler scope_fail with throwing function object") {
  std::ostringstream out{};
  throwing_copy c{"called because of exception!!!", out};
  REQUIRE_THROWS(scope_fail(c));
  REQUIRE("called because of exception!!!\n" == out.str());
}

TEST_CASE("Test throw on unique_resource doesn't crash it") {
  std::ostringstream out{};
  {
    auto guard = unique_resource(1, [&](auto) { out << "done\n"; });
    // we do no longer allow that for unique_resource
    //        try {
    //            {
    //                auto guard1 = unique_resource(2, [] (auto) noexcept(false) {throw 42;});
    //                guard1.reset();
    //            }
    //            FAILM("didn't throw");
    //        } catch (int) {
    //        } catch (...) {
    //            FAILM("threw unknown error");
    //        }
  }
  REQUIRE("done\n" == out.str());
}

TEST_CASE("Test unique_resource simple") {
  using namespace std;
  std::ostringstream out{};

  const std::string msg{" deleted resource\n"};
  {
    auto res = unique_resource(std::ref(msg), [&out](string msg) { out << msg << flush; });
  }
  REQUIRE(msg == out.str());
}

TEST_CASE("Test unique_resource by reference") {
  using namespace std;
  std::ostringstream out{};

  const std::string msg{" deleted resource\n"};
  {
    auto res = unique_resource<std::string const &, std::function<void(string)>>(msg, [&out](string msg) {
      out << msg << flush;
    });
  }
  REQUIRE(msg == out.str());
}

TEST_CASE("Test unique_resource semantics reset") {
  std::ostringstream out{};
  {
    auto cleanup = unique_resource(1, [&out](int i) { out << "cleaned " << i; });
    cleanup.reset(2);
  }
  REQUIRE("cleaned 1cleaned 2" == out.str());
}

TEST_CASE("Demonstrate unique_resource with stdio") {
  const std::string filename = "hello.txt";
  auto fclose                = [](auto fptr) { ::fclose(fptr); };
  {
    auto file = unique_resource(::fopen(filename.c_str(), "w"), fclose);
    ::fputs("Hello World!\n", file.get());
    REQUIRE(file.get() != NULL);
  }
  {
    std::ifstream input{filename};
    std::string line{};
    getline(input, line);
    REQUIRE("Hello World!" == line);
    getline(input, line);
    REQUIRE(input.eof());
  }
  ::unlink(filename.c_str());
  {
    auto file = make_unique_resource_checked(::fopen("nonexistentfile.txt", "r"), nullptr, fclose);
    REQUIRE((FILE *)NULL == file.get());
  }
}

TEST_CASE("Demonstrate unique_resource with stdio cpp17") {
  const std::string filename = "hello.txt";
  auto fclose                = [](auto fptr) { ::fclose(fptr); };

  {
    unique_resource file(::fopen(filename.c_str(), "w"), fclose);
    ::fputs("Hello World!\n", file.get());
    REQUIRE(file.get() != NULL);
  }
  {
    std::ifstream input{filename};
    std::string line{};
    getline(input, line);
    REQUIRE("Hello World!" == line);
    getline(input, line);
    REQUIRE(input.eof());
  }
  ::unlink(filename.c_str());
  {
    auto file = make_unique_resource_checked(::fopen("nonexistentfile.txt", "r"), nullptr, fclose);
    REQUIRE((FILE *)NULL == file.get());
  }
}

TEST_CASE("Demontrate unique_resource with posix io") {
  const std::string filename = "./hello1.txt";
  auto close                 = [](auto fd) { ::close(fd); };
  {
    auto file = unique_resource(::open(filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666), close);

    ::write(file.get(), "Hello World!\n", 12u);
    REQUIRE(file.get() != -1);
  }
  {
    std::ifstream input{filename};
    std::string line{};
    getline(input, line);
    REQUIRE("Hello World!" == line);
    getline(input, line);
    REQUIRE(input.eof());
  }
  ::unlink(filename.c_str());
  {
    auto file = make_unique_resource_checked(::open("nonexistingfile.txt", O_RDONLY), -1, close);
    REQUIRE(-1 == file.get());
  }
}

TEST_CASE("Demontrate unique_resource with posix io lvalue") {
  const std::string filename = "./hello1.txt";
  auto close                 = [](auto fd) { ::close(fd); };
  {
    auto fd = ::open(filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);

    auto file = make_unique_resource_checked(fd, -1, close);
    REQUIRE(fd != -1);
    ::write(file.get(), "Hello World!\n", 12u);
    REQUIRE(file.get() != -1);
  }
  {
    std::ifstream input{filename};
    std::string line{};
    getline(input, line);
    REQUIRE("Hello World!" == line);
    getline(input, line);
    REQUIRE(input.eof());
  }
  ::unlink(filename.c_str());
  {
    auto fd   = ::open("nonexistingfile.txt", O_RDONLY);
    auto file = make_unique_resource_checked(fd, -1, close);
    REQUIRE(-1 == file.get());
  }
}

TEST_CASE("Test make_unique_resource_checked") {
  std::ostringstream out{};

  {
    auto bar = make_unique_resource_checked(0.0, 0, [&out](auto i) { out << i << "not called"; });
    auto foo = make_unique_resource_checked(0.0, -1, [&out](auto) { out << "called"; });
  }
  REQUIRE("called" == out.str());
}

struct throwing_copy_movable_resource {
  throwing_copy_movable_resource(const char *sz, std::ostream &os)
      : sz_{sz}
      , out{os} {}
  throwing_copy_movable_resource(const throwing_copy_movable_resource &other)
      : out{other.out} {
    using namespace std::literals;
    throw "throwing_copy copy attempt:"s + sz_;
  }
  throwing_copy_movable_resource(throwing_copy_movable_resource &&other) noexcept = default;
  bool operator==(int i) const & {
    return i == 0;
  }
  friend std::ostream &operator<<(std::ostream &os, throwing_copy_movable_resource const &t) {
    return os << t.sz_;
  }

private:
  const char *sz_{""};
  std::ostream &out;
};

bool operator==(std::reference_wrapper<throwing_copy_movable_resource> const &, int i) {
  return i == 0;
}

TEST_CASE("Test make_unique_resource_checked lvalue") {
  std::ostringstream out{};

  {
    // throwing_copy r{"",out}; // this does not work, make_unique_resource_checked only works for copies
    auto r{0LL};
    auto bar = make_unique_resource_checked(r, 0, [&out](auto &) { out << "not called"; });
    auto foo = make_unique_resource_checked(r, 1, [&out](auto &) { out << "called\n"; });
  }
  REQUIRE("called\n" == out.str());
}

TEST_CASE("Test make_unique_resource_checked failing copy") {
  std::ostringstream out{};
  {
    throwing_copy_movable_resource x{"x by ref ", out};
    auto bar = make_unique_resource_checked(std::ref(x), 0, [&out](auto const &i) { out << i.get() << "not called"; });
    auto foo = make_unique_resource_checked(std::ref(x), 1, [&out](auto const &i) { out << i.get() << "called\n"; });
  }
  throwing_copy x{"x by value", out};
  try {
    auto bar = make_unique_resource_checked(x, 0, [&out](auto const &i) { out << i << "not called"; });
  } catch (...) {
    out << "expected\n";
  }
  try {
    auto bar = make_unique_resource_checked(x, 1, [&out](auto const &i) { out << i << " called\n"; });
  } catch (...) {
    out << "expected2\n";
  }
  REQUIRE("x by ref called\nexpected\nx by value called\nexpected2\n" == out.str());
}

TEST_CASE("Test make_unique_resource_checked with deleter throwing on copy") {
  std::ostringstream out{};
  REQUIRE_THROWS(make_unique_resource_checked(0, 0, throwing_copy("notcalled", out)));
  REQUIRE_THROWS(make_unique_resource_checked(42, 0, throwing_copy("called ", out)));
  REQUIRE("called 42\n" == out.str());
}

TEST_CASE("Test reference_wrapper") {
  std::ostringstream out{};
  int i{42};
  {
    auto uref = unique_resource(std::ref(i), [&out](int &j) { out << "reference to " << j++; });
  }
  REQUIRE("reference to 42" == out.str());
  REQUIRE(43 == i);
}

void TalkToTheWorld(std::ostream &out, const std::string farewell = "Uff Wiederluege...") {
  // Always say goodbye before returning,
  // but if given a non-empty farewell message use it...
  auto goodbye = scope_exit([&out]() { out << "Goodbye world..." << std::endl; });
  // must pass farewell by reference, otherwise it is not non-throw-moveconstructible
  auto altgoodbye = scope_exit([&out, &farewell]() { out << farewell << std::endl; });

  if (farewell.empty()) {
    altgoodbye.release(); // Don't use farewell!
  } else {
    goodbye.release(); // Don't use the alternate
  }
}

TEST_CASE("Test TalkToTheWorld") {
  std::ostringstream out{};
  TalkToTheWorld(out, "");
  REQUIRE("Goodbye world...\n" == out.str());
  out.str("");
  TalkToTheWorld(out);
  REQUIRE("Uff Wiederluege...\n" == out.str());
}

struct X {
  void foo() const {}
};

TEST_CASE("Test compilability guard for non pointer unique resource") {
  auto x = unique_resource(X{}, [](X) {});
  unique_resource y(X{}, [](X) {});
#if defined(CHECK_COMPILE_ERRORS)
  x->foo(); // compile error!
  *x;       // compile error!
  y->foo();
  *y;
#endif
}
TEST_CASE("Test compilability guard for pointer types") {
  auto x = unique_resource(new int{42}, [](int *ptr) { delete ptr; });
  REQUIRE(42 == *x);
  auto y = unique_resource(new X{}, [](X *ptr) { delete ptr; });
  y->foo(); // compiles, SFINAE works
  (void)*y; // compiles, through SFINAE (again)
  REQUIRE(42 == *(int *)(void *)x.get());
}

struct functor_copy_throws {
  functor_copy_throws() = default;
  functor_copy_throws(const functor_copy_throws &) noexcept(false) {
    throw 42;
  }
  functor_copy_throws(functor_copy_throws &&) = default;
  void operator()() {}
};
struct functor_move_throws { // bad idea anyway.
  functor_move_throws() = default;
  functor_move_throws(functor_move_throws &&) noexcept(false) {
    throw 42;
  }
  functor_move_throws(const functor_move_throws &) = default;
  void operator()() const {}
};

struct functor_move_copy_throws {
  functor_move_copy_throws() = default;
  functor_move_copy_throws(functor_move_copy_throws &&) noexcept(false) {
    throw 42;
  }
  functor_move_copy_throws(const functor_move_copy_throws &) noexcept(false) {
    throw 42;
  }
  void operator()() const {}
};

TEST_CASE("Test scope_exit with throwing fun copy") {
  functor_copy_throws fun{};
#if defined(CHECK_COMPILE_ERRORS)
  auto x         = scope_exit(std::move(fun)); // doesn't compile due to static_assert
  auto x1        = scope_exit(42);             // not callable
  auto const &ff = functor_copy_throws{};
  auto z         = scope_exit(std::ref(ff)); // hold by const reference
#endif
  auto y = scope_exit(std::ref(fun)); // hold by reference
}

TEST_CASE("Test scope_exit with throwing fun move") {
  functor_move_throws fun{};
  const functor_move_throws &funref{fun};
  // #if defined(CHECK_COMPILE_ERRORS)
  auto x = scope_exit(std::move(fun)); // no longer a compile error, because it is copied.
                                       // #endif
  auto y = scope_exit(fun);            // hold by copy
  auto z = scope_exit(funref);         // hold by copy?, no const ref
}

TEST_CASE("Test scope_exit with throwing fun move and copy") {
  functor_move_copy_throws fun{};
#if defined(CHECK_COMPILE_ERRORS)
  auto x = scope_exit(std::move(fun)); // compile error, because non-copyable
#endif
  auto y = scope_exit(std::ref(fun)); // hold by reference works
}

TEST_CASE("Test scope_success with side effect") {
  std::ostringstream out{};
  auto lam = [&] { out << "not called"; };
  try {
    auto x = scope_success(lam); // lam not called
    throw 42;
  } catch (...) {
    auto y = scope_success([&] { out << "handled"; });
  }
  REQUIRE("handled" == out.str());
}

TEST_CASE("Test scope_success might throw") {
  std::ostringstream out{};
  auto lam = [&] {
    out << "called";
    throw 42;
  };
  REQUIRE_THROWS_AS(scope_success(lam), int);
  REQUIRE("called" == out.str());
}

TEST_CASE("Demo scope_exit, scope_fail, and scope_success") {
  std::ostringstream out{};
  auto lam = [&] { out << "called "; };
  try {
    auto v = scope_exit([&] { out << "always "; });
    auto w = scope_success([&] { out << "not "; }); // not called
    auto x = scope_fail(lam);                       // called
    throw 42;
  } catch (...) {
    auto y = scope_fail([&] { out << "not "; });       // not called
    auto z = scope_success([&] { out << "handled"; }); // called
  }
  REQUIRE("called always handled" == out.str());
}

TEST_CASE("Demo scope_exit, scope_fail, and scope_success C++17") {
  std::ostringstream out{};
  auto lam = [&] { out << "called "; };
  try {
    scope_exit v([&] { out << "always "; });
    scope_success w([&] { out << "not "; }); // not called
    scope_fail x(lam);                       // called
    throw 42;
  } catch (...) {
    scope_fail y([&] { out << "not "; });       // not called
    scope_success z([&] { out << "handled"; }); // called
  }
  REQUIRE("called always handled" == out.str());
}

TEST_CASE("Test scope_exit lvalue ref passing rvalue fails to compile") {
  using fun = void (*)();
  auto y    = scope_exit<const fun &>(fun(nullptr)); // no static assert needed. fails to match.
  y.release();                                       // avoid crash from calling nullptr
  scope_exit<const fun &> z(fun(nullptr));
  z.release(); // avoid crash from calling nullptr
  scope_exit zz(fun(nullptr));
  zz.release();
#if defined(CHECK_COMPILE_ERRORS)
  auto x = scope_exit<fun &>(fun(nullptr));                    // no static assert needed. fails to match
  std::experimental::scope_exit<const fun &> se{fun(nullptr)}; // static assert needed
  scope_exit se17{fun(nullptr)};                               // static assert needed
#endif
}

struct nasty {};

struct deleter_2nd_throwing_copy {
  deleter_2nd_throwing_copy() = default;
  deleter_2nd_throwing_copy(deleter_2nd_throwing_copy const &) {
    if (copied % 2) {
      throw nasty{};
    }
    ++copied;
  }
  void operator()(int const &) const {
    ++deleted;
  }
  static inline int deleted{};
  static inline int copied{};
};

TEST_CASE("Test sometimes throwing deleter copy ctor") {
  using uid = unique_resource<int, deleter_2nd_throwing_copy>;
  uid strange{1, deleter_2nd_throwing_copy{}};
  REQUIRE(0 == deleter_2nd_throwing_copy::deleted);

  strange.release();
  REQUIRE(0 == deleter_2nd_throwing_copy::deleted);

  REQUIRE_THROWS_AS(uid{std::move(strange)}, nasty);
  REQUIRE(0 == deleter_2nd_throwing_copy::deleted);
  REQUIRE(1 == deleter_2nd_throwing_copy::copied);
}

namespace {
std::string just_for_testing_side_effect{};
}

void functocall() {
  just_for_testing_side_effect.append("functocall_called\n");
}

TEST_CASE("Test that explicit reference parameter compiles") {
  {
    scope_exit<void (&)()> guard{functocall};
    scope_exit guardfromptr{functocall};
  }
  REQUIRE("functocall_called\nfunctocall_called\n" == just_for_testing_side_effect);
}

TEST_CASE("Macros shall expand to the correct function call and declare different variable names") {
  auto stream = std::stringstream{};
  {
    SCOPE_EXIT([&stream] { stream << "exit\n"; });
    try {
      SCOPE_FAIL([&stream] { stream << "fail\n"; });
      throw 42;
    } catch (...) {
    }
    SCOPE_SUCCESS([&stream] { stream << "success\n"; });
  }
  REQUIRE(stream.str() == "fail\nsuccess\nexit\n");
}
