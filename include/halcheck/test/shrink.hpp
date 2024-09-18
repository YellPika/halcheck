#ifndef HALCHECK_TEST_SHRINK_HPP
#define HALCHECK_TEST_SHRINK_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

namespace halcheck { namespace test {

test::strategy shrink();
test::strategy forward_shrink();

}} // namespace halcheck::test

#endif
