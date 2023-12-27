#ifndef HALCHECK_TEST_CHECK_HPP
#define HALCHECK_TEST_CHECK_HPP

#include <halcheck/test/limited.hpp>
#include <halcheck/test/random.hpp>
#include <halcheck/test/shrinking.hpp>

namespace halcheck { namespace test {

static constexpr auto check = test::shrinking(test::limited(test::random()));

}} // namespace halcheck::test

#endif
