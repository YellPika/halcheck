#ifndef HALCHECK_TEST_SERIALIZE_HPP
#define HALCHECK_TEST_SERIALIZE_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <string>

namespace halcheck { namespace test {

struct write_effect {
  std::string key;
  std::string value;
  void fallback() const {}
};

static const auto write = [](std::string key, std::string value) {
  eff::invoke<write_effect>(std::move(key), std::move(value));
};

test::strategy serialize(std::string name, std::string folder = lib::getenv("HALCHECK_FOLDER").value_or(".halcheck"));

}} // namespace halcheck::test

#endif
