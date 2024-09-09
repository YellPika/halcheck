#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

namespace gen = halcheck::gen;
namespace lib = halcheck::lib;

template<typename TypeParam>
struct Range : public testing::Test {};

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
TYPED_TEST_SUITE(Range, Types, );

HALCHECK_TYPED_TEST(Range, Range) {
  using namespace lib::literals;

  TypeParam min = gen::arbitrary("min"_s);
  TypeParam max = gen::arbitrary("max"_s);
  auto value = gen::range("value"_s, min, max);
  EXPECT_LE(min, value) << "max: " << max;
  EXPECT_LT(value, max) << "min: " << min;
}
