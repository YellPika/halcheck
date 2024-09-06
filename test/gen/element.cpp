#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <algorithm>
#include <vector>

using namespace halcheck;

HALCHECK_TEST(ElementOf, Reference) {
  using namespace lib::literals;

  auto xs = gen::label("xs"_s, gen::arbitrary<std::vector<char>>);
  auto &x = gen::element_of("x"_s, xs);
  EXPECT_EQ(std::count_if(xs.begin(), xs.end(), [&](char &y) { return &x == &y; }), 1)
      << "xs: " << ::testing::PrintToString(xs) << "\n"
      << "x: " << ::testing::PrintToString(x);
}

HALCHECK_TEST(Element, Example) {
  auto x = gen::element(1, 2, 3, 4, 5);
  EXPECT_LE(1, x);
  EXPECT_LE(x, 5);
}
