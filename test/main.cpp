#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>
#include <halcheck/gtest/pp.hpp>
#include <halcheck/test/strategy.hpp>

#include <random>

using namespace halcheck;

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc, argv);
  FLAGS_logbuflevel = -1;

  struct handler : lib::effect::handler<handler, gtest::default_strategy_effect> {
    test::strategy operator()(gtest::default_strategy_effect) override {
      auto test = ::testing::UnitTest::GetInstance();
      auto info = test->current_test_info();
      auto name = info->test_suite_name() + std::string(".") + info->name();
      auto seed = std::mt19937_64(test->random_seed());
      return test::config(test::set("SEED", seed, true)) | // Use GTest's seed by default
             glog::filter(                                 // Only show output from the last failing test case
                 (test::deserialize(name)                  // Run saved test cases first
                  & test::serialize(name))                 // Then save new test cases
                 | test::random()                          // Produce test cases randomly
                 | test::shrink());                        // Shrink failing test cases
    }
  };

  auto _ = handler().handle();
  return RUN_ALL_TESTS();
}
