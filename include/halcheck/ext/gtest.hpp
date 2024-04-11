#ifndef HALCHECK_EXT_GTEST_HPP
#define HALCHECK_EXT_GTEST_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/test/capture.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/replay.hpp>

#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>

#include "gtest/gtest.h"
#include <exception>
#include <memory>

namespace halcheck { namespace ext { namespace gtest {

template<typename Strategy = decltype(test::check)>
void check(void (*func)(), const char * = "", Strategy strategy = test::check) {
  using namespace testing;

  static const char *replay = std::getenv("HALCHECK_REPLAY");
  static std::string dir = replay ? std::string(replay) : ".";

  static const char *hex = "0123456789abcdef";
  std::string name(UnitTest::GetInstance()->current_test_info()->name());
  std::string filename(dir + "/halcheck-gtest-");
  filename.reserve(filename.size() + name.size() * 2);
  for (char ch : name) {
    filename.push_back(hex[ch & 0xF]);
    filename.push_back(hex[(ch & 0xF0) >> 4]);
  }

  struct failure {
    std::unique_ptr<TestPartResultArray> results;
  };

  auto body = [&] {
    std::unique_ptr<TestPartResultArray> results(new TestPartResultArray);
    {
      ScopedFakeTestPartResultReporter reporter(
          ScopedFakeTestPartResultReporter::INTERCEPT_ONLY_CURRENT_THREAD, results.get());
      func();
    }
    if (results->size() > 0) {
      auto &part = results->GetTestPartResult(0);
      if (part.skipped()) {
        GTEST_MESSAGE_AT_(part.file_name(), part.line_number(), part.message(), part.type());
        gen::succeed();
      } else {
        throw failure{std::move(results)};
      }
    }
  };

  try {
    if (dir.empty())
      lib::invoke(std::move(strategy), std::move(body));
    else
      lib::invoke(test::capture(test::replay(std::move(strategy), filename), filename), std::move(body));
  } catch (const failure &e) {
    for (std::size_t i = 0; i < e.results->size(); i++) {
      auto &part = e.results->GetTestPartResult(i);
      GTEST_MESSAGE_AT_(part.file_name(), part.line_number(), part.message(), part.type());
    }
  }
}

}}} // namespace halcheck::ext::gtest

#define HALCHECK_EMPTY
#define HALCHECK_EXPAND(x) x

#define HALCHECK_CAT_HELPER(x, y) x##y
#define HALCHECK_CAT(x, y) HALCHECK_CAT_HELPER(x, y)

#define HALCHECK_1ST_HELPER(x, ...) x
#define HALCHECK_1ST(...) HALCHECK_EXPAND(HALCHECK_1ST_HELPER(__VA_ARGS__, HALCHECK_EMPTY))

#define HALCHECK_TEST_HELPER(anon, suite, ...)                                                                         \
  static void anon();                                                                                                  \
  TEST(suite, HALCHECK_1ST(__VA_ARGS__)) {                                                                             \
    const char *HALCHECK_1ST(__VA_ARGS__) = "";                                                                        \
    ::halcheck::ext::gtest::check(anon, __VA_ARGS__);                                                                  \
  }                                                                                                                    \
  static void anon()

#define HALCHECK_TEST(suite, ...) HALCHECK_TEST_HELPER(HALCHECK_CAT(HALCHECK_ANON_FUNC_, __LINE__), suite, __VA_ARGS__)

#define HALCHECK_TEST_F_HELPER(anon, fixture, ...)                                                                     \
  static void anon();                                                                                                  \
  TEST_F(fixture, HALCHECK_1ST(__VA_ARGS__)) {                                                                         \
    const char *HALCHECK_1ST(__VA_ARGS__) = "";                                                                        \
    ::halcheck::ext::gtest::check(anon, __VA_ARGS__);                                                                  \
  }                                                                                                                    \
  static void anon()

#define HALCHECK_TEST_F(fixture, ...)                                                                                  \
  HALCHECK_TEST_F_HELPER(HALCHECK_CAT(HALCHECK_ANON_FUNC_, __LINE__), fixture, __VA_ARGS__)

#endif
