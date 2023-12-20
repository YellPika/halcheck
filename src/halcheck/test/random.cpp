#include "halcheck/test/random.hpp"

#include <halcheck/test/sampler.hpp>

using namespace halcheck;

static_assert(test::is_sampler<test::random>(), "IMPOSSIBLE");
