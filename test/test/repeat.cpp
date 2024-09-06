#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <cstddef>
#include <ostream>

using namespace halcheck;

HALCHECK_TEST(Test, Repeat_Discard) {
  using namespace lib::literals;

  // If we always discard, we should eventually hit the limit.
  auto max_success = gen::range("max_success"_s, 1, 200);
  auto max_discard_ratio = gen::range("max_discard_ratio"_s, 1, 20);
  ASSERT_THROW(
      (test::repeat(max_success, max_discard_ratio))([] { throw gen::discard(); }),
      test::discard_limit_exception);
}

HALCHECK_TEST(Test, Repeat_OK) {
  using namespace lib::literals;

  // If we never discard, we should eventually succeed.
  auto max_success = gen::range("max_success"_s, 1, 200);
  auto max_discard_ratio = gen::range("max_discard_ratio"_s, 0, 20);
  LOG(INFO) << "max_success: " << max_success;
  LOG(INFO) << "max_discard_ratio: " << max_discard_ratio;
  eff::reset([&] {
    std::size_t i = 0;
    (test::repeat(max_success, max_discard_ratio) | gtest::wrap())([&] { EXPECT_LT(i++, max_success); });
  });
}

HALCHECK_TEST(Test, Repeat_Infinite) {
  using namespace lib::literals;

  // If we set no discard limit, then we should be able to iterate an arbitrary number of times.
  auto max_success = gen::range("max_success"_s, 0, 200);
  auto count = gen::range("count"_s, 0, 2000);
  (test::repeat(max_success, 0))([&] {
    if (count-- > 0)
      throw gen::discard();
    else
      throw gen::succeed();
  });
}
