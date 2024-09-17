#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <algorithm>
#include <vector>

using namespace halcheck;

static void check_reference() {
  using namespace lib::literals;
  auto xs = gen::arbitrary<std::vector<char>>("xs"_s);
  auto &x = gen::element_of("x"_s, xs);
  EXPECT_EQ(std::count_if(xs.begin(), xs.end(), [&](char &y) { return &x == &y; }), 1)
      << "xs: " << ::testing::PrintToString(xs) << "\n"
      << "x: " << ::testing::PrintToString(x);
}

HALCHECK_TEST(ElementOf, Reference_Shrinks) {
  auto value = gen::make_shrinks(check_reference);
  for (std::size_t i = 0; !value.children().empty(); i++)
    value = gen::make_shrinks(gen::element_of(lib::number(i), value.children()), check_reference);
}

HALCHECK_TEST(ElementOf, Reference_ForwardShrinks) {
  auto value = gen::make_forward_shrinks(check_reference);
  for (std::size_t i = 0; !value.children().empty(); i++)
    value = gen::make_forward_shrinks(gen::element_of(lib::number(i), value.children()), check_reference);
}

HALCHECK_TEST(Element, Example) {
  using namespace lib::literals;
  auto x = gen::element("x"_s, 1, 2, 3, 4, 5);
  EXPECT_LE(1, x);
  EXPECT_LE(x, 5);
}
