#ifndef HALCHECK_EXT_DOCTEST_HPP
#define HALCHECK_EXT_DOCTEST_HPP

#include <halcheck/fmt/show.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/test/check.hpp>

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

namespace halcheck { namespace ext { namespace doctest {
template<typename T>
::doctest::String stringify(const T &value) {
  auto output = fmt::to_string(value);
  return ::doctest::String(output.data(), output.size());
}

inline ::doctest::String stringify(const char *value) { return value; }

template<typename Sampler = decltype(test::check)>
void check(void (*func)(), const char *, Sampler sampler = test::check) {
  lib::invoke(sampler, func);
}
}}} // namespace halcheck::ext::doctest

#define HALCHECK_1ST_HELPER(x, ...) x
#define HALCHECK_1ST(...) HALCHECK_1ST_HELPER(__VA_ARGS__, HALCHECK_DUMMY)

#define HALCHECK_2ND_HELPER(x, ...) HALCHECK_1ST(__VA_ARGS__)
#define HALCHECK_2ND(...) HALCHECK_2ND_HELPER(__VA_ARGS__, HALCHECK_DUMMY)

#define HALCHECK_TEST_CASE_HELPER(anon, ...)                                                                           \
  static void anon();                                                                                                  \
  TEST_CASE(HALCHECK_1ST(__VA_ARGS__)) { HALCHECK_2ND(__VA_ARGS__, ::halcheck::test::check)(anon); }                   \
  static void anon()

#define HALCHECK_TEST_CASE(...) HALCHECK_TEST_CASE_HELPER(DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_), __VA_ARGS__)

#define HALCHECK_TEST_CASE_TEMPLATE_DEFINE_HELPER(anon, dec, T, ...)                                                   \
  template<typename T>                                                                                                 \
  static void anon();                                                                                                  \
  TEST_CASE_TEMPLATE_DEFINE(dec, T, HALCHECK_1ST(__VA_ARGS__)) {                                                       \
    HALCHECK_2ND(__VA_ARGS__, ::halcheck::test::check)(anon<T>);                                                       \
  }                                                                                                                    \
  template<typename T>                                                                                                 \
  static void anon()

#define HALCHECK_TEST_CASE_TEMPLATE_DEFINE(dec, T, ...)                                                                \
  HALCHECK_TEST_CASE_TEMPLATE_DEFINE_HELPER(DOCTEST_ANONYMOUS(HALCHECK_ANON_FUNC_), dec, T, __VA_ARGS__)

#endif
