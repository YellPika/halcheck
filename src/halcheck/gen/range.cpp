#include "halcheck/gen/range.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/limited.hpp>
#include <halcheck/test/random.hpp>
#include <halcheck/test/shrinking.hpp>

#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#endif

using namespace halcheck;

TEST_CASE_TEMPLATE_DEFINE("gen::range", T, gen_range) {
  test::limited(test::random(), 1 << 16)([] {
    auto min = gen::arbitrary<T>();
    CAPTURE(min);

    auto max = gen::arbitrary<T>();
    CAPTURE(max);

    auto value = gen::range(min, max);
    CAPTURE(value);

    REQUIRE_LE(min, value);
    REQUIRE_LT(value, max);

    std::clog << min << " ≤ " << value << " < " << max << "\n";
  });
}

TEST_SUITE("gen::range") {
  TEST_CASE_TEMPLATE_INVOKE(gen_range, char);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, signed char);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, unsigned char);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, signed short);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, unsigned short);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, signed int);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, unsigned int);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, signed long);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, unsigned long);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, signed long long);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, unsigned long long);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, float);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, double);
  TEST_CASE_TEMPLATE_INVOKE(gen_range, long double);
}
