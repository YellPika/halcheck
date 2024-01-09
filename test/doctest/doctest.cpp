#include "halcheck/ext/doctest.hpp"

using namespace doctest;

namespace {
struct halcheck_listener final : IReporter {
  halcheck_listener(const ContextOptions &) {}

  void log_assert(const AssertData &data) override {
    if (data.m_failed)
      ++halcheck::ext::doctest::failures();
  }

  void test_case_start(const TestCaseData &) override {}
  void test_case_reenter(const TestCaseData &) override {}
  void report_query(const QueryData &) override {}
  void test_run_start() override {}
  void test_run_end(const TestRunStats &) override {}
  void test_case_end(const CurrentTestCaseStats &) override {}
  void test_case_exception(const TestCaseException &) override {}
  void subcase_start(const SubcaseSignature &) override {}
  void subcase_end() override {}
  void log_message(const MessageData &) override {}
  void test_case_skipped(const TestCaseData &) override {}
};
} // namespace

int &halcheck::ext::doctest::failures() {
  static int value = 0;
  return value;
}

REGISTER_LISTENER("halcheck_listener", 1, halcheck_listener);
