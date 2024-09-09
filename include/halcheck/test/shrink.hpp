#ifndef HALCHECK_TEST_SHRINK_HPP
#define HALCHECK_TEST_SHRINK_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>

namespace halcheck { namespace test {

test::strategy shrink(std::uintmax_t repetitions = lib::getenv<std::uintmax_t>("HALCHECK_REPETITIONS").value_or(1));
test::strategy
forward_shrink(std::uintmax_t repetitions = lib::getenv<std::uintmax_t>("HALCHECK_REPETITIONS").value_or(1));

}} // namespace halcheck::test

#endif
