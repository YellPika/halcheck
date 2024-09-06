#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

namespace gen = halcheck::gen;
namespace lib = halcheck::lib;
namespace test = halcheck::test;

template<typename TypeParam>
struct Numeric : public testing::Test {};

using Types = testing::Types<
    char,
    unsigned char,
    signed char,
    unsigned char,
    signed short,
    unsigned short,
    signed int,
    unsigned long,
    signed long,
    float,
    double,
    long double>;
TYPED_TEST_SUITE(Numeric, Types, );

HALCHECK_TYPED_TEST(Numeric, Midpoint) {
  using namespace lib::literals;

  auto x = gen::label("x"_s, gen::arbitrary<TypeParam>);
  auto y = gen::label("y"_s, gen::arbitrary<TypeParam>);
  auto mid = lib::midpoint(x, y);

  auto min = std::min(x, y);
  EXPECT_GE(mid, min);

  auto max = std::max(x, y);
  EXPECT_LE(mid, max);
}
