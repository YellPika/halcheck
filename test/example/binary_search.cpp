#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include "gtest/gtest.h"

#include <vector>

using namespace halcheck;
using namespace halcheck::lib::literals;

namespace {
bool binary_search(const std::vector<int> &xs, int key) {
  std::size_t max = xs.size(), min = 0;
  while (min < max) {
    auto mid = lib::midpoint(min, max);
    if (key < xs[mid])
      max = mid - 1;
    else if (key > xs[mid])
      min = mid + 1;
    else
      return true;
  }
  return false;
}
} // namespace

TEST(BinarySearch, Examples) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  EXPECT_FALSE(binary_search({}, 0));
  EXPECT_TRUE(binary_search({0}, 0));
  EXPECT_TRUE(binary_search({0, 1, 2}, 2));
  EXPECT_TRUE(binary_search({0, 1, 2, 3, 4, 5}, 1));
}

HALCHECK_TEST(BinarySearch, Membership) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  auto xs = gen::container<std::vector<int>>("xs"_s, gen::arbitrary<int>);
  std::sort(xs.begin(), xs.end());

  auto x = gen::element_of("x"_s, xs);

  LOG(INFO) << "xs: " << testing::PrintToString(xs);
  LOG(INFO) << "x: " << testing::PrintToString(x);

  EXPECT_TRUE(binary_search(xs, x));
}
