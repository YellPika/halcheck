#include "halcheck/test/check.hpp" // IWYU pragma: keep

#include <halcheck/test/random.hpp>
#include <halcheck/test/repeat.hpp>
#include <halcheck/test/shrink.hpp>
#include <halcheck/test/sized.hpp>
#include <halcheck/test/strategy.hpp>

using namespace halcheck;

test::strategy test::check() { return test::repeat() | test::random() | test::sized() | test::shrink(); }
