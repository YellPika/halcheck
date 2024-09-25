#ifndef HALCHECK_TEST_CONFIG_HPP
#define HALCHECK_TEST_CONFIG_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/strategy.hpp>

#include <initializer_list>
#include <string>
#include <type_traits>
#include <utility>

namespace halcheck { namespace test {

struct set {
  template<typename T>
  set(std::string key, const T &value) : key(std::move(key)), value(lib::to_string(value)) {}

  std::string key;
  std::string value;
};

test::strategy config(const std::initializer_list<test::set> &);

template<typename... Args, HALCHECK_REQUIRE(lib::conjunction<std::is_convertible<Args, test::set>...>())>
test::strategy config(Args &&...args) {
  return test::config({test::set(std::forward<Args>(args))...});
}

}} // namespace halcheck::test

#endif
