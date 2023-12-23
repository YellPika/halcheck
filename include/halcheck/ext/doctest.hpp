#ifndef HALCHECK_EXT_DOCTEST_HPP
#define HALCHECK_EXT_DOCTEST_HPP

namespace doctest {
struct String;
} // namespace doctest

namespace halcheck { namespace ext { namespace doctest {
template<typename T>
::doctest::String stringify(const T &);
inline ::doctest::String stringify(const char *);
}}} // namespace halcheck::ext::doctest

#define DOCTEST_STRINGIFY(...) ::halcheck::ext::doctest::stringify(__VA_ARGS__)

#include <halcheck/fmt/show.hpp>

#include <doctest/doctest.h>

namespace halcheck { namespace ext { namespace doctest {
template<typename T>
::doctest::String stringify(const T &value) {
  auto output = fmt::to_string(value);
  return ::doctest::String(output.data(), output.size());
}

inline ::doctest::String stringify(const char *value) { return value; }
}}} // namespace halcheck::ext::doctest

#endif
