#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <glog/logging.h>

#include <cstdint>
#include <functional>
#include <future>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

using namespace halcheck;

HALCHECK_TEST(Shrink, To) {
  using namespace lib::literals;

  using T = std::uint8_t;
  auto dst = gen::label("dst"_s, gen::arbitrary<T>);
  auto src = gen::label("src"_s, gen::arbitrary<T>);

  auto func = [&] { return gen::shrink_to(dst, src); };
  auto prev = gen::label("eval"_s, gen::shrinks, std::ref(func));
  ASSERT_EQ(prev.get(), src);
  while (prev.children().begin() != prev.children().end()) {
    auto offset = gen::range("offset"_s, 0, prev.size());
    auto next = gen::label("eval"_s, gen::shrinks, *std::next(prev.children().begin(), long(offset)), std::ref(func));
    if (dst < src) {
      ASSERT_LE(dst, next.get());
      ASSERT_LT(next.get(), prev.get());
    } else {
      ASSERT_GE(dst, next.get());
      ASSERT_GT(next.get(), prev.get());
    }
    prev = std::move(next);
  }
}

HALCHECK_TEST(Shrink, Example) {
  using namespace lib::literals;

  using T = uint8_t;

  auto threshold = gen::range("threshold"_s, 0, std::numeric_limits<T>::max());
  auto size = gen::range("size"_s, 1UL, 10UL);
  auto index = gen::range("index"_s, 0UL, size);

  std::vector<T> expected(size, T(0));
  expected[index] = threshold;
  LOG(INFO) << "expected: " << testing::PrintToString(expected);

  try {
    eff::reset([&] {
      (test::repeat(0, 0) | test::random() | test::sized(100) | test::shrink())([&] {
        auto xs = gen::arbitrary<std::vector<T>>();
        if (xs.size() >= size && xs[index] >= threshold) {
          LOG(INFO) << "xs: " << testing::PrintToString(xs);
          throw xs; // NOLINT
        }
      });
    });

    FAIL() << "failure not caught!";
  } catch (const std::vector<T> &e) {
    ASSERT_EQ(e, expected);
  }
}

TEST(Test, Shrink_Concurrency) {
  using namespace lib::literals;

  eff::reset([&] {
    (test::repeat() | test::random() | test::shrink() | gtest::wrap())([&] {
      auto func = [] {
        auto x = gen::shrink();
        auto y = gen::shrink("a"_s);
        return std::make_pair(x, y);
      };
      auto future = std::async(std::launch::async, eff::wrap(func));
      EXPECT_EQ(func(), future.get());
    });
  });
}