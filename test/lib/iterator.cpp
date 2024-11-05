#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <vector>

TEST(Filter, Example1) {
  using namespace halcheck;

  std::vector<int> input = {0, 1, 3, 4, 6, 9};
  auto is_even = [](int i) { return i % 2 == 0; };
  auto begin = lib::make_filter_iterator(input.begin(), input.end(), is_even);
  auto end = lib::make_filter_iterator(input.end(), input.end(), is_even);
  const std::vector<int> actual(begin, end);
  const std::vector<int> expected = {0, 4, 6};
  EXPECT_EQ(actual, expected);
}

TEST(Filter, Example2) {
  using namespace halcheck;

  auto is_even = [](int i) { return i % 2 == 0; };
  auto add_one = [](int i) { return i + 1; };
  auto ibegin = lib::make_iota_iterator(0);
  auto iend = lib::make_iota_iterator(10);
  auto fbegin = lib::make_filter_iterator(ibegin, iend, is_even);
  auto fend = lib::make_filter_iterator(iend, iend, is_even);
  auto begin = lib::make_transform_iterator(fbegin, add_one);
  auto end = lib::make_transform_iterator(fend, add_one);
  const std::vector<int> actual(begin, end);
  const std::vector<int> expected = {1, 3, 5, 7, 9};
  EXPECT_EQ(actual, expected);
}
