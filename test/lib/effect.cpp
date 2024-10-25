#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <gtest/gtest.h>

#include <map>
#include <ostream>
#include <stdexcept>

using namespace halcheck;
using namespace halcheck::lib::literals;

namespace {

template<int N>
struct example {
  int fallback() const { throw std::runtime_error("unhandled example"); }
};

void seq(lib::atom id, const std::map<int, int> &ref);

void noop(lib::atom, const std::map<int, int> &ref) { LOG(INFO) << "noop(" << testing::PrintToString(ref) << ")"; }

template<int I>
void invoke(lib::atom id, const std::map<int, int> &ref) {
  LOG(INFO) << "BEGIN invoke<" << I << ">(" << testing::PrintToString(ref) << ")";

  auto _1 = gen::label(id);
  auto it = ref.find(I);
  if (it != ref.end())
    EXPECT_EQ(lib::effect::invoke<example<I>>(), it->second);
  else
    EXPECT_THROW(lib::effect::invoke<example<I>>(), std::runtime_error);

  LOG(INFO) << "END invoke<" << I << ">";
}

template<int I>
void handle(lib::atom id, const std::map<int, int> &ref) {
  LOG(INFO) << "BEGIN handle<" << I << ">(" << testing::PrintToString(ref) << ")";

  auto _0 = gen::label(id);

  struct handler : lib::effect::handler<handler, example<I>> {
    explicit handler(std::map<int, int> ref) : ref(std::move(ref)), result(gen::arbitrary<int>("result"_s)) {}

    int operator()(example<I>) final {
      auto _ = gen::scale(0.3);
      seq("seq"_s, ref);
      return result;
    }

    std::map<int, int> ref;
    int result;
  } handler(ref);

  auto _1 = handler.handle();

  auto copy = ref;
  copy[I] = handler.result;

  auto _2 = gen::scale(0.3);
  seq("seq"_s, copy);

  LOG(INFO) << "END handle<" << I << ">";
}

void seq(lib::atom id, const std::map<int, int> &ref) {
  auto _ = gen::label(id);

  using namespace lib::literals;
  for (auto _ : gen::repeat("commands"_s)) {
    gen::element(
        "command"_s,
        &noop,
        &invoke<0>,
        &handle<0>,
        &noop,
        &invoke<1>,
        &handle<1>,
        &noop,
        &invoke<2>,
        &handle<2>)("command"_s, ref);
  }
}

} // namespace

HALCHECK_TEST(Effect, Model) {
  using namespace lib::literals;
  seq("seq"_s, std::map<int, int>());
}

TEST(Effect, Example) {
  EXPECT_THROW(lib::effect::invoke<example<1>>(), std::runtime_error);
  EXPECT_THROW(lib::effect::invoke<example<2>>(), std::runtime_error);

  struct handler1 : lib::effect::handler<handler1, example<1>> {
    int operator()(example<1>) final {
      EXPECT_THROW(lib::effect::invoke<example<1>>(), std::runtime_error);
      EXPECT_THROW(lib::effect::invoke<example<2>>(), std::runtime_error);
      return 1;
    }
  } handler1;

  handler1.handle([&] {
    struct handler2 : lib::effect::handler<handler2, example<2>> {
      int operator()(example<2>) final {
        EXPECT_EQ(lib::effect::invoke<example<1>>(), 1);
        EXPECT_THROW(lib::effect::invoke<example<2>>(), std::runtime_error);
        throw std::logic_error("OH NO");
      }
    } handler2;
    EXPECT_EQ(lib::effect::invoke<example<1>>(), 1);
    EXPECT_THROW(lib::effect::invoke<example<2>>(), std::runtime_error);
    handler2.handle([&] {
      EXPECT_EQ(lib::effect::invoke<example<1>>(), 1);
      EXPECT_THROW(lib::effect::invoke<example<2>>(), std::logic_error);
    });
  });

  EXPECT_THROW(lib::effect::invoke<example<1>>(), std::runtime_error);
  EXPECT_THROW(lib::effect::invoke<example<2>>(), std::runtime_error);
}
