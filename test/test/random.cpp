#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <cstddef>
#include <future>
#include <ostream>
#include <random>
#include <stdexcept>
#include <utility>

using namespace halcheck;

HALCHECK_TEST(Test, Random_Error) {
  using namespace lib::literals;

  std::mt19937_64 seed(gen::sample("seed"_s));
  auto _ = lib::effect::state().handle();

  EXPECT_THROW(
      (test::config(test::set("SEED", seed), test::set("MAX_SUCCESS", 0)) |
       test::random())([&] {
        auto x = gen::sample("a"_s);
        LOG(INFO) << "x: " << x;
        switch (x % 8) {
        case 0:
          throw gen::discard_exception();
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
  auto config = test::config(test::set("MAX_SUCCESS", max_success), test::set("DISCARD_RATIO", discard_ratio));

  lib::effect::state().handle([&] {
    ASSERT_THROW(
        (std::move(config) | test::random())([] { throw gen::discard_exception(); }),
        test::discard_limit_exception);
  });
}

HALCHECK_TEST(Test, Random_OK) {
  using namespace lib::literals;

  // If we never discard, we should eventually succeed.
  auto max_success = gen::range("max_success"_s, 1, 200);
  auto discard_ratio = gen::range("discard_ratio"_s, 0, 20);
  auto config = test::config(test::set("MAX_SUCCESS", max_success), test::set("DISCARD_RATIO", discard_ratio));
  LOG(INFO) << "max_success: " << max_success;
  LOG(INFO) << "discard_ratio: " << discard_ratio;

  lib::effect::state().handle([&] {
    std::size_t i = 0;
    gtest::wrap(std::move(config) | test::random())([&] {
      LOG(INFO) << "i: " << i;
      EXPECT_LT(i++, max_success);
    });
  });
}

HALCHECK_TEST(Test, Random_Infinite) {
  using namespace lib::literals;

  // If we set no discard limit, then we should be able to iterate an arbitrary number of times.
  auto config =
      test::config(test::set("MAX_SUCCESS", gen::range("max_success"_s, 0, 200)), test::set("DISCARD_RATIO", 0));
  auto count = gen::range("count"_s, 0, 2000);

  lib::effect::state().handle([&] {
    (std::move(config) | test::random())([&] {
      if (count-- > 0)
        throw gen::discard_exception();
      else
        gen::succeed();
    });
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
    auto context = lib::effect::save();
    auto future = std::async(std::launch::async, [&] { return context.handle(func); });
    EXPECT_EQ(func(), future.get());
  });
}
