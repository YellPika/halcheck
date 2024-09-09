#ifndef HALCHECK_TEST_REPEAT_HPP
#define HALCHECK_TEST_REPEAT_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <exception>

namespace halcheck { namespace test {

struct discard_limit_exception : std::exception {
  const char *what() const noexcept override { return "discard limit reached"; /*GCOVR_EXCL_LINE*/ }
};

test::strategy repeat(
    std::uintmax_t max_success = lib::getenv<std::uintmax_t>("HALCHECK_MAX_SUCCESS").value_or(100),
    std::uintmax_t max_discard_ratio = lib::getenv<std::uintmax_t>("HALCHECK_MAX_DISCARD_RATIO").value_or(10));

}} // namespace halcheck::test

#endif
