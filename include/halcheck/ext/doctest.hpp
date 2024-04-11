#ifndef HALCHECK_EXT_DOCTEST_HPP
#define HALCHECK_EXT_DOCTEST_HPP

#include <halcheck/fmt/show.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/test/capture.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/replay.hpp>

#include <exception>
#include <string>

namespace doctest {
struct String;
} // namespace doctest

namespace halcheck { namespace ext { namespace doctest {
template<typename T>
::doctest::String stringify(const T &);
inline ::doctest::String stringify(const char *);
}}} // namespace halcheck::ext::doctest

#define DOCTEST_STRINGIFY(...) ::halcheck::ext::doctest::stringify(__VA_ARGS__)

#include <doctest/doctest.h>

#ifdef DOCTEST_CONFIG_DISABLE
#define HALCHECK_TEST_CASE(...)                                                                                        \
  template<typename HALCHECK_UNUSED_TEMPLATE_TYPE>                                                                     \
  static inline void DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_)()
#define HALCHECK_TEST_CASE_TEMPLATE_DEFINE(name, T, ...)                                                               \
  template<typename T>                                                                                                 \
  static inline void DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_)()
#else
namespace halcheck { namespace ext { namespace doctest {
int &failures();

template<typename T>
::doctest::String stringify(const T &value) {
  auto output = fmt::to_string(value);
  return ::doctest::String(output.data(), output.size());
}

inline ::doctest::String stringify(const char *value) { return value; }

struct assert_error : std::exception {
  const char *what() const noexcept override { return "Assertion failures detected"; }
};

template<typename Strategy = decltype(test::check)>
void check(void (*func)(), const char *, Strategy strategy = test::check) {
  static const char *replay = std::getenv("HALCHECK_REPLAY");
  static std::string dir = replay ? std::string(replay) : ".";

  static const char *hex = "0123456789abcdef";
  auto test = ::doctest::getContextOptions()->currentTest;
  auto name = std::string(test->m_file.c_str()) + ":" + std::to_string(test->m_line);
  std::string filename(dir + "/halcheck-doctest-");
  filename.reserve(filename.size() + name.size() * 2);
  for (char ch : name) {
    filename.push_back(hex[ch & 0xF]);
    filename.push_back(hex[(ch & 0xF0) >> 4]);
  }

  auto body = [&] {
    failures() = 0;
    func();
    if (failures() > 0)
      throw assert_error();
  };

  if (dir.empty())
    lib::invoke(std::move(strategy), std::move(body));
  else
    lib::invoke(test::capture(test::replay(std::move(strategy), filename), filename), std::move(body));
}
}}} // namespace halcheck::ext::doctest

#define HALCHECK_EMPTY
#define HALCHECK_EXPAND(x) x

#define HALCHECK_1ST_HELPER(x, ...) x
#define HALCHECK_1ST(...) HALCHECK_EXPAND(HALCHECK_1ST_HELPER(__VA_ARGS__, HALCHECK_EMPTY))

#define HALCHECK_TEST_CASE_HELPER(anon, ...)                                                                           \
  static void anon();                                                                                                  \
  TEST_CASE(HALCHECK_1ST(__VA_ARGS__)) { ::halcheck::ext::doctest::check(anon, __VA_ARGS__); }                         \
  static void anon()

#define HALCHECK_TEST_CASE(...) HALCHECK_TEST_CASE_HELPER(DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_), __VA_ARGS__)

#define HALCHECK_TEST_CASE_TEMPLATE_DEFINE_HELPER(anon, dec, T, ...)                                                   \
  template<typename T>                                                                                                 \
  static void anon();                                                                                                  \
  TEST_CASE_TEMPLATE_DEFINE(dec, T, HALCHECK_1ST(__VA_ARGS__)) {                                                       \
    const char *HALCHECK_1ST(__VA_ARGS__) = "";                                                                        \
    ::halcheck::ext::doctest::check(anon<T>, __VA_ARGS__);                                                             \
  }                                                                                                                    \
  template<typename T>                                                                                                 \
  static void anon()

#define HALCHECK_TEST_CASE_TEMPLATE_DEFINE(dec, T, ...)                                                                \
  HALCHECK_TEST_CASE_TEMPLATE_DEFINE_HELPER(DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_), dec, T, __VA_ARGS__)

#endif
#endif
