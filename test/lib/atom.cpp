#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

TEST(Symbol, Equality) {
  using namespace halcheck;
  using namespace lib::literals;

  lib::symbol a = "hello"_s;
  lib::symbol b = "hello"_s;
  lib::symbol c = "world"_s;
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(b, c);
}
