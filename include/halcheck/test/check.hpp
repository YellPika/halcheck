#ifndef HALCHECK_TEST_CHECK_HPP
#define HALCHECK_TEST_CHECK_HPP

#include <halcheck/test/limit.hpp>
#include <halcheck/test/random.hpp>
#include <halcheck/test/shrink.hpp>

namespace halcheck { namespace test {

static constexpr auto check = test::limit(test::shrink(test::random()));

}} // namespace halcheck::test

#endif
