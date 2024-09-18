#ifndef HALCHECK_TEST_DESERIALIZE_HPP
#define HALCHECK_TEST_DESERIALIZE_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <initializer_list>
#include <string>

namespace halcheck { namespace test {

struct read_effect {
  std::string key;
  lib::optional<std::string> fallback() const { return lib::getenv("HALCHECK_" + key); }
};

inline lib::optional<std::string> read(std::string key) { return eff::invoke<read_effect>(std::move(key)); }

template<typename T>
inline lib::optional<T> read(std::string key) {
  auto value = test::read(std::move(key));
  return value ? lib::of_string<T>(std::move(*value)) : lib::nullopt;
}

test::strategy deserialize(std::string name);
test::strategy override(const std::initializer_list<std::pair<std::string, std::string>> &);

}} // namespace halcheck::test

#endif
