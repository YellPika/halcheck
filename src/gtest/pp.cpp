#include "halcheck/gtest/pp.hpp"

#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/random.hpp>
#include <halcheck/test/repeat.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/shrink.hpp>
#include <halcheck/test/sized.hpp>
#include <halcheck/test/strategy.hpp>

#include "gtest/gtest.h"

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
         | test::shrink();                            // Shrink failing test cases
}
