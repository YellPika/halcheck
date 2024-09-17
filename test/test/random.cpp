#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <future>

using namespace halcheck;

// HALCHECK_TEST(Test, Random_Error) {
//   struct error {};
//   EXPECT_THROW(
//       test::random(gen::arbitrary<std::uintmax_t>())([&] {
//         switch (gen::sample() % 8) {
//         case 0:
//           throw gen::discard();
//           return;
//         case 1:
//           gen::shrink();
//           return;
//         case 2:
//           gen::label("A", [] {});
//           return;
//         default:
//           throw error();
//         }
//       }),
//       error);
// }

// HALCHECK_TEST(Test, Random_Succeed) {
//   auto i = gen::range(0, 1000);
//   test::random()([&] {
//     if (i-- == 0)
//       throw gen::succeed();
//   });
// }

TEST(Test, Random_Concurrency) {
  (test::random() | gtest::wrap())([&] {
    using namespace lib::literals;

    auto func = [] {
      auto x = gen::sample("x"_s);
      auto y = gen::sample("y"_s);
      return std::make_pair(x, y);
    };
    auto future = std::async(std::launch::async, eff::wrap(func));
    EXPECT_EQ(func(), future.get());
  });
}
