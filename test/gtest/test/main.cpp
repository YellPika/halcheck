#include <halcheck/fmt/show.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#define DOCTEST_CONFIG_IMPLEMENT

#include <halcheck/ext/gtest.hpp>
#include <halcheck/gen/arbitrary.hpp>

using namespace halcheck;

TEST(gtest, example) {
  EXPECT_NONFATAL_FAILURE(
      halcheck::ext::gtest::check([] {
        auto xs = gen::arbitrary<std::vector<bool>>();
        EXPECT_LE(xs.size(), 10);
      }),
      "");
}

HALCHECK_TEST(gtest, skip) { GTEST_SKIP(); }

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
