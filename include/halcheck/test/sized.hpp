#ifndef HALCHECK_TEST_SIZED_HPP
#define HALCHECK_TEST_SIZED_HPP

#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>

namespace halcheck { namespace test {

test::strategy sized(std::uintmax_t max_size = lib::getenv<std::uintmax_t>("HALCHECK_MAX_SIZE").value_or(100));

}} // namespace halcheck::test

#endif
