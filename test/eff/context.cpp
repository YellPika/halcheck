#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

// #include <map>
// #include <ostream>
#include <stdexcept>

namespace eff = halcheck::eff;
namespace lib = halcheck::lib;

namespace {

template<int N>
struct example {
  int fallback() const { return eff::invoke<example<N - 1>>(); }
};

template<>
struct example<0> {
  int fallback() const { throw std::runtime_error("unhandled example"); }
};

// using ref = std::map<int, int>;
// using sys = lib::monostate;
// using cmd = state::Command<ref, sys>;

// #define SET(i) noop, invoke<i>, handle<i>
// #define COMMANDS SET(0), SET(1), SET(2), SET(3)

// struct noop final : cmd {};

// template<int I>
// struct invoke final : cmd {
//   void run(const ref &ref, sys &) const override {
//     auto it = ref.find(I);
//     if (it != ref.end())
//       RC_ASSERT(eff::invoke<example<I>>() == it->second);
//     else
//       RC_ASSERT_THROWS_AS(RC_LOG() << eff::invoke<example<I>>(), std::runtime_error);
//   }

//   void show(std::ostream &os) const override { os << "invoke<" << I << ">"; }
// };

// template<int I>
// struct handle final : cmd {
//   state::Commands<cmd> func;
//   int result = 0;
//   state::Commands<cmd> body;

//   handle() = default;

//   explicit handle(const ref &ref) {
//     func = *gen::scale(0.3, state::gen::commands(ref, state::gen::execOneOfWithArgs<COMMANDS>()));
//     result = *gen::arbitrary<int>();

//     auto copy = ref;
//     copy[I] = result;
//     body = *gen::scale(0.3, state::gen::commands(copy, state::gen::execOneOfWithArgs<COMMANDS>()));
//   }

//   void run(const ref &ref, sys &sys) const override {
//     auto copy = ref;
//     copy[I] = result;

//     struct handler : eff::handler<handler, example<I>> {
//       handler(const handle *self, const ::ref &ref, ::sys &sys, int result)
//           : self(self), ref(&ref), sys(&sys), result(result) {}

//       int operator()(example<I>) override {
//         state::runAll(self->func, *ref, *sys);
//         return result;
//       }

//       const handle *self;
//       const ::ref *ref;
//       ::sys *sys;
//       int result;
//     } handler(this, ref, sys, result);

//     eff::handle([&] { state::runAll(body, copy, sys); }, handler);
//   }

//   void show(std::ostream &os) const override {
//     os << "handle<" << I << ">{func = " << rc::toString(func) << ", result = " << result
//        << ", body = " << rc::toString(body) << "}";
//   }
// };

} // namespace

// RC_GTEST_PROP(Effect, Model, ()) {
//   sys s0;
//   state::check(ref(), s0, state::gen::execOneOfWithArgs<COMMANDS>());
// }

TEST(Effect, Example) {
  EXPECT_THROW(eff::invoke<example<0>>(), std::runtime_error);
  EXPECT_THROW(eff::invoke<example<1>>(), std::runtime_error);

  struct handler : eff::handler<handler, example<0>, example<1>> {
    int operator()(example<0>) override {
      EXPECT_THROW(eff::invoke<example<0>>(), std::runtime_error);
      EXPECT_THROW(eff::invoke<example<1>>(), std::runtime_error);
      return 0;
    }

    int operator()(example<1>) override {
      EXPECT_THROW(eff::invoke<example<0>>(), std::runtime_error);
      EXPECT_THROW(eff::invoke<example<1>>(), std::runtime_error);
      return 1;
    }
  } handler;

  eff::handle(
      [&] {
        EXPECT_EQ(eff::invoke<example<0>>(), 0);
        EXPECT_EQ(eff::invoke<example<1>>(), 1);
      },
      handler);

  EXPECT_THROW(eff::invoke<example<0>>(), std::runtime_error);
  EXPECT_THROW(eff::invoke<example<1>>(), std::runtime_error);
}
