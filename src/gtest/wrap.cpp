#include "halcheck/gtest/wrap.hpp" // IWYU pragma: keep

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/test/strategy.hpp>

#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

#include <memory>
#include <stdexcept>
#include <utility>

using namespace halcheck;

test::strategy gtest::wrap() {
  using namespace testing;

  return [](const std::function<void()> &func) {
    TestPartResultArray results;
    auto _ = lib::finally([&] {
      for (int i = 0; i < results.size(); i++) {
        if (results.GetTestPartResult(i).skipped())
          GTEST_SKIP();
      }
    });

    try {
      auto old = lib::exchange(GTEST_FLAG(throw_on_failure), true);
      auto _ = lib::finally([&] { GTEST_FLAG(throw_on_failure) = old; });
      ScopedFakeTestPartResultReporter reporter(ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, &results);
      func();
    } catch (const testing::internal::GoogleTestFailureException &e) {
      const auto &result = results.GetTestPartResult(results.size() - 1);
      GTEST_MESSAGE_AT_(result.file_name(), result.line_number(), result.message(), result.type());
    }
  };
}
