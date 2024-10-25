#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

using namespace halcheck;

test::strategy gtest::default_strategy() {
  auto test = ::testing::UnitTest::GetInstance();
  auto info = test->current_test_info();
  auto name = info->test_suite_name() + std::string(".") + info->name();
  return glog::filter(          // Only show output from the last failing test case
      (test::deserialize(name)  // Run saved test cases first
       & test::serialize(name)) // Then save new test cases
      | test::random()          // Produce test cases randomly
      | test::shrink());        // Shrink failing test cases
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc, argv);
  FLAGS_logbuflevel = -1;
  return RUN_ALL_TESTS();
}
