#ifndef HALCHECK_TEST_DESERIALIZE_HPP
#define HALCHECK_TEST_DESERIALIZE_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <string>

namespace halcheck { namespace test {

struct read_effect {
  std::string key;
  lib::optional<std::string> fallback() const { return lib::nullopt; }
};

static const auto read = [](std::string key) { return eff::invoke<read_effect>(std::move(key)); };

test::strategy deserialize(std::string name, std::string folder = lib::getenv("HALCHECK_FOLDER").value_or(".halcheck"));

}} // namespace halcheck::test

#endif
