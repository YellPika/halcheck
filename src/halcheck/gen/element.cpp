#include "halcheck/gen/element.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/shrinking.hpp>

#include <algorithm>
#include <vector>

using namespace halcheck;

TEST_SUITE("gen::element") {
  TEST_CASE("gen::element returns a reference to an element") {
    test::check([] {
      std::vector<int> xs;
      while (gen::next(1, gen::weight::current - xs.size()))
        xs.push_back(gen::arbitrary<int>());

      auto &x = gen::element(xs);
      auto i = std::find(xs.begin(), xs.end(), x);
      REQUIRE_EQ(&x, &*i);
    });
  }

  TEST_CASE("gen::element works with std::initializer_list") {
    test::check([] {
      auto x = gen::element({1, 2, 3, 4, 5});
      REQUIRE_LE(1, x);
      REQUIRE_LE(x, 5);
    });
  }
}
