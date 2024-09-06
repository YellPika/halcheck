#ifndef HALCHECK_TEST_RANDOM_HPP
#define HALCHECK_TEST_RANDOM_HPP

#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>

namespace halcheck { namespace test {

test::strategy random(std::int_fast32_t seed = lib::getenv<std::int_fast32_t>("HALCHECK_SEED").value_or(0));

}} // namespace halcheck::test

#endif
