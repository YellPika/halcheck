#ifndef HALCHECK_TEST_SERIALIZE_HPP
#define HALCHECK_TEST_SERIALIZE_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <string>
#include <type_traits>

namespace halcheck { namespace test {

struct write_effect {
  std::string key;
  std::string value;
  void fallback() const {}
};

template<typename T, HALCHECK_REQUIRE(!std::is_convertible<T, std::string>())>
void write(std::string key, const T &value) {
  lib::effect::invoke<write_effect>(std::move(key), lib::to_string(value));
}

inline void write(std::string key, std::string value) {
  lib::effect::invoke<write_effect>(std::move(key), std::move(value));
}

test::strategy serialize(std::string name);

}} // namespace halcheck::test

#endif
