#include <halcheck.hpp>
#include <halcheck/gtest.hpp>
#include <stdexcept>

using namespace halcheck;

TEST(Sample, Throws) { EXPECT_THROW(gen::sample(), std::runtime_error); }

// TEST(Sample, With) {
//   auto strategy = [](std::uintmax_t) { return true; };
//   gen::sample.with(strategy);
//   gen::sample.with(std::move(strategy));
// }
