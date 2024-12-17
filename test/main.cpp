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
      return glog::filter(gtest::default_strategy());
    }
  };

  auto _ = handler().handle();
  return RUN_ALL_TESTS();
}
