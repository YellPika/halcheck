#include "halcheck/test/check.hpp" // IWYU pragma: keep

#include <halcheck/test/random.hpp>
#include <halcheck/test/shrink.hpp>
#include <halcheck/test/strategy.hpp>

using namespace halcheck;

test::strategy test::check() { return test::random() | test::shrink(); }
