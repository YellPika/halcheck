// #include <halcheck.hpp>
// #include <halcheck/gtest.hpp>

// using namespace halcheck;

// TEST(Label, Example) {
//   struct example_t : lib::eff<int, example_t> {
//     static int fallback() { return 42; }
//   } example;

//   int x, y, i = 0;
//   lib::tree<lib::atom, lib::optional<int>> tree;

//   lib::handle(
//       [&] {
//         gen::label.record(example, tree, [&] {
//           x = example();
//           EXPECT_EQ(tree.get(), x);

//           y = gen::label(HALCHECK_ATOM("A"), example);
//           EXPECT_EQ(tree.child(HALCHECK_ATOM("A")).get(), y);
//         });
//       },
//       example.with([&] { return i++; }));

//   gen::label.replay(example, tree, [&] {
//     EXPECT_EQ(example(), x);
//     EXPECT_EQ(gen::label(HALCHECK_ATOM("A"), example), y);
//     EXPECT_EQ(gen::label(HALCHECK_ATOM("B"), example), 42);
//   });
// }
