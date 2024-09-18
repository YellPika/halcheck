#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <cstddef>
#include <future>
#include <ostream>
#include <stdexcept>
#include <utility>

using namespace halcheck;

HALCHECK_TEST(Test, Random_Error) {
  using namespace lib::literals;

  EXPECT_THROW(
      test::random()([&] {
        switch (gen::sample("a"_s) % 8) {
        case 0:
          throw gen::discard();
          return;
        case 1:
          gen::shrink("b"_s);
          return;
        case 2:
          gen::label("A"_s, [] {});
          return;
        default:
          throw std::runtime_error("example");
        }
      }),
      std::runtime_error);
}

HALCHECK_TEST(Test, Random_Discard) {
  using namespace lib::literals;

  // If we always discard, we should eventually hit the limit.
  auto max_success = gen::range("max_success"_s, 1, 200);
  auto discard_ratio = gen::range("discard_ratio"_s, 1, 20);
  auto config = test::override({
      {"MAX_SUCCESS",   lib::to_string(max_success)  },
      {"DISCARD_RATIO", lib::to_string(discard_ratio)}
  });

  auto _ = eff::reset();
  ASSERT_THROW((std::move(config) | test::random())([] { throw gen::discard(); }), test::discard_limit_exception);
}

HALCHECK_TEST(Test, Random_OK) {
  using namespace lib::literals;

  // If we never discard, we should eventually succeed.
  auto max_success = gen::range("max_success"_s, 1, 200);
  auto discard_ratio = gen::range("discard_ratio"_s, 0, 20);
  auto config = test::override({
      {"MAX_SUCCESS",   lib::to_string(max_success)  },
      {"DISCARD_RATIO", lib::to_string(discard_ratio)}
  });
  LOG(INFO) << "max_success: " << max_success;
  LOG(INFO) << "discard_ratio: " << discard_ratio;

  auto _ = eff::reset();
  std::size_t i = 0;
  gtest::wrap(std::move(config) | test::random())([&] {
    LOG(INFO) << "i: " << i;
    EXPECT_LT(i++, max_success);
  });
}

HALCHECK_TEST(Test, Random_Infinite) {
  using namespace lib::literals;

  // If we set no discard limit, then we should be able to iterate an arbitrary number of times.
  auto config = test::override({
      {"MAX_SUCCESS", lib::to_string(gen::range("max_success"_s, 0, 200))},
      {"DISCARD_RATIO", "0"}
  });
  auto count = gen::range("count"_s, 0, 2000);

  auto _ = eff::reset();
  (std::move(config) | test::random())([&] {
    if (count-- > 0)
      throw gen::discard();
    else
      throw gen::succeed();
  });
}

TEST(Test, Random_Concurrency) {
  gtest::wrap(test::random())([&] {
    using namespace lib::literals;

    auto func = [] {
      auto x = gen::sample("x"_s);
      auto y = gen::sample("y"_s);
      return std::make_pair(x, y);
    };
    auto future = std::async(std::launch::async, eff::wrap(func));
    EXPECT_EQ(func(), future.get());
  });
}
