#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <vector>

TEST(Filter, Example) {
  using namespace halcheck;

  auto input = {0, 1, 3, 4, 6, 9};
  auto is_even = [](int i) { return i % 2 == 0; };
  auto begin = lib::make_filter_iterator(input.begin(), input.end(), is_even);
  auto end = lib::make_filter_iterator(input.end(), input.end(), is_even);
  std::vector<int> actual(begin, end);
  std::vector<int> expected = {0, 4, 6};
  EXPECT_EQ(actual, expected);
}
