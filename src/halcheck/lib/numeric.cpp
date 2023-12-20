#include "halcheck/lib/numeric.hpp"

#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/test/check.hpp>

#include <doctest/doctest.h>

using namespace halcheck;

TEST_CASE("lib::midpoint") {
  test::check([] {
    auto min = gen::arbitrary<std::int16_t>();
    auto max = gen::arbitrary<std::int16_t>();
    auto mid = lib::midpoint(min, max);
    CAPTURE(min);
    CAPTURE(max);
    CAPTURE(mid);

    auto d1 = std::abs(mid - min);
    auto d2 = std::abs(max - mid);
    auto d3 = std::abs(max - min);
    REQUIRE_EQ(d1 + d2, d3);
    REQUIRE_GE(d2, d1);
  });
}
