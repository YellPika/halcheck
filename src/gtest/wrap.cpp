#include "halcheck/gtest/wrap.hpp" // IWYU pragma: keep

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"

#include <exception>

using namespace halcheck;
using namespace testing;

namespace {
struct error : std::exception {
  explicit error(std::unique_ptr<TestPartResultArray> results) noexcept : results(std::move(results)) {}
  const char *what() const noexcept override { return results->GetTestPartResult(0).summary(); }
  std::unique_ptr<TestPartResultArray> results;
};

struct strategy {
  void operator()(lib::function_view<void()> func) const {
    try {
      inner([&] {
        TestPartResultArray results;
        auto check_skip = [&] {
          for (int i = 0; i < results.size(); i++) {
            if (results.GetTestPartResult(i).skipped()) {
              [] { GTEST_SKIP(); }();
              gen::succeed();
              throw gen::discard_exception();
            }
          }
        };

        try {
          auto old = lib::exchange(GTEST_FLAG(throw_on_failure), true);
          auto _ = lib::finally([&] { GTEST_FLAG(throw_on_failure) = old; });
          ScopedFakeTestPartResultReporter reporter(ScopedFakeTestPartResultReporter::INTERCEPT_ALL_THREADS, &results);
          func();
        } catch (...) {
          check_skip();
          throw;
        }

        check_skip();
      });
    } catch (const internal::GoogleTestFailureException &e) {
      GTEST_FAIL() << "\n" << e.what();
    }
  }

  test::strategy inner;
};
} // namespace

test::strategy gtest::wrap(test::strategy inner) { return strategy{std::move(inner)}; }
