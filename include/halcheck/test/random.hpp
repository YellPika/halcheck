#ifndef HALCHECK_TEST_RANDOM_HPP
#define HALCHECK_TEST_RANDOM_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include <exception>

namespace halcheck { namespace test {

struct discard_limit_exception : std::exception {
  const char *what() const noexcept override { return "discard limit reached"; /*GCOVR_EXCL_LINE*/ }
};

test::strategy random();

}} // namespace halcheck::test

#endif
