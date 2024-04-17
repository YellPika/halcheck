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

HALCHECK_TEST(gtest, binary_search, test::limit(test::random(), 1024)) {
  using limits = std::numeric_limits<int8_t>;
  auto min = gen::range(limits::min(), limits::max());
  auto max = int8_t(gen::range(min, limits::max()) + 1);
  auto err = gen::range(min, max);
  std::clog << "min: " << fmt::show(min) << std::endl;
  std::clog << "max: " << fmt::show(max) << std::endl;
  std::clog << "err: " << fmt::show(err) << std::endl;
  try {
    test::check([&] {
      auto x = gen::range(min, max);
      if (x >= err) {
        std::clog << "> x: " << fmt::show(x) << std::endl;
        throw x;
      }
    });
  } catch (const decltype(err) &e) {
    EXPECT_EQ(e, err);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
