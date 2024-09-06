#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <string>

using namespace halcheck;

test::strategy gtest::default_strategy() {
  auto test = ::testing::UnitTest::GetInstance();
  auto info = test->current_test_info();
  auto name = info->test_suite_name() + std::string(".") + info->name();
  return (test::deserialize(name)                     // Run saved test cases first
          & (test::repeat() | test::serialize(name))) // Then run (and save) new test cases repeatedly
         | test::random(test->random_seed())          // Produce test cases randomly
         | test::sized()                              // Produce test cases of increasing size
         | test::shrink()                             // Shrink failing test cases
         | glog::filter();                            // Only show output from the last failing test case
}

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc, argv);
  FLAGS_logbuflevel = -1;
  return RUN_ALL_TESTS();
}
