#include "halcheck/lib/effect.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/element.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <functional>
#include <vector>

using namespace halcheck;

thread_local std::array<lib::context::frame, lib::context::size> lib::context::stack;
thread_local std::size_t lib::context::top = 0;
std::atomic_size_t lib::context::next{0};

static void effect_test(std::vector<std::pair<std::reference_wrapper<lib::effect<char>>, char>> state = {}) {
  enum command { CALL, HANDLE, CREATE };
  auto i = gen::size();
  while (gen::next(1, i--)) {
    switch (gen::weighted<command>({{4 * !state.empty(), CALL}, {2 * !state.empty(), HANDLE}, {1, CREATE}})) {

    case CALL: {
      auto &&pair = gen::element(state);
      CHECK_EQ(pair.first.get()(), pair.second);
    } break;

    case HANDLE: {
      auto value = gen::arbitrary<char>();
      auto &&pair = gen::element(state);
      auto _0 = pair.first.get().handle([=] {
        auto _ = gen::size.handle(gen::size() / 4);
        effect_test(state);
        return value;
      });

      std::swap(pair.second, value);
      auto _1 = gen::size.handle(gen::size() / 4);
      effect_test(state);
      std::swap(pair.second, value);
    } break;

    case CREATE: {
      auto value = gen::arbitrary<char>();
      lib::effect<char> eff([=] {
        auto _ = gen::size.handle(gen::size() / 4);
        effect_test(state);
        return value;
      });

      state.emplace_back(std::ref(eff), value);
      auto _ = gen::size.handle(gen::size() / 4);
      effect_test(state);
      state.pop_back();
    } break;
    }
  }
}

HALCHECK_TEST_CASE("lib::effect") { effect_test(); }

TEST_CASE("effect example") {
  lib::effect<bool> example0([] { return (throw 0, true); });
  lib::effect<bool> example1([] { return true; });

  CHECK_THROWS_AS(example0(), int);
  CHECK(example1());

  {
    auto _0 = example0.handle([&] {
      CHECK_THROWS_AS(example0(), int);
      return true;
    });

    auto _1 = example1.handle([&] {
      CHECK(example1());
      CHECK(example0());
      return false;
    });

    CHECK(example0());
    CHECK_FALSE(example1());
  }

  CHECK_THROWS_AS(example0(), int);
  CHECK(example1());
}
