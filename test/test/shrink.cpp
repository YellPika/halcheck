#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <cstdint>
#include <future>
#include <limits>
#include <ostream>
#include <vector>

using namespace halcheck;

HALCHECK_TEST(Shrink, To) {
  using namespace lib::literals;

  using T = std::uint8_t;

  auto dst = gen::arbitrary<T>("dst"_s);
  auto src = gen::arbitrary<T>("src"_s);

  auto func = [&] { return gen::shrink_to("eval"_s, dst, src); };
  auto prev = gen::make_shrinks(func);
  ASSERT_EQ(prev.get(), src);
  while (prev.children().begin() != prev.children().end()) {
    auto child = gen::element_of("child"_s, prev.children());
    auto next = gen::make_shrinks(std::move(child), func);
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

  auto threshold = gen::range("threshold"_s, 0, (std::numeric_limits<T>::max)());
  auto size = gen::range("size"_s, 1UL, 10UL);
  auto index = gen::range("index"_s, 0UL, size);

  std::vector<T> expected(size, T(0));
  expected[index] = threshold;
  LOG(INFO) << "expected: " << testing::PrintToString(expected);

  try {
    lib::effect::state().handle([&] {
      (test::config(test::set("MAX_SUCCESS", 0)) | test::random() | test::shrink())([&] {
        auto xs = gen::arbitrary<std::vector<T>>("xs"_s);
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

HALCHECK_TEST(ForwardShrink, Example) {
  using namespace lib::literals;

  using T = uint8_t;

  auto threshold = gen::range("threshold"_s, 0, (std::numeric_limits<T>::max)());
  LOG(INFO) << "threshold: " << threshold;

  auto size = gen::range("size"_s, 1UL, 10UL);
  LOG(INFO) << "size: " << size;

  auto index = gen::range("index"_s, 0UL, size);
  LOG(INFO) << "index: " << index;

  std::vector<T> expected(size, T(0));
  expected[index] = threshold;
  LOG(INFO) << "expected: " << testing::PrintToString(expected);

  try {
    lib::effect::state().handle([&] {
      (test::config(test::set("MAX_SUCCESS", 0)) | test::random() | test::forward_shrink())([&] {
        auto xs = gen::arbitrary<std::vector<T>>("xs"_s);
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

  lib::effect::state().handle([&] {
    gtest::wrap(test::random() | test::shrink())([&] {
      auto func = [] {
        auto x = gen::shrink("x"_s);
        auto y = gen::shrink("y"_s);
        return std::make_pair(x, y);
      };
      auto context = lib::effect::save();
      auto future = std::async(std::launch::async, [&] { return context.handle(func); });
      EXPECT_EQ(func(), future.get());
    });
  });
}
