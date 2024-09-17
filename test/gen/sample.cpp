#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <stdexcept>

using namespace halcheck;

TEST(Sample, Throws) {
  using namespace lib::literals;
  EXPECT_THROW(gen::sample("a"_s), std::runtime_error);
}

// TEST(Sample, With) {
//   auto strategy = [](std::uintmax_t) { return true; };
//   gen::sample.with(strategy);
//   gen::sample.with(std::move(strategy));
// }
