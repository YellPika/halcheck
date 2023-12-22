#include "halcheck/test/random.hpp"

#include <halcheck/test/strategy.hpp>

using namespace halcheck;

static_assert(test::is_sampler<test::random>(), "IMPOSSIBLE");
