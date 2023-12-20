#include "halcheck/lib/effect.hpp"

#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/element.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <doctest/doctest.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

using namespace halcheck;

thread_local std::array<lib::context::frame, lib::context::size> lib::context::stack;
thread_local std::size_t lib::context::top = 0;
std::atomic_size_t lib::context::next{0};

static void effect_test(
    std::vector<std::pair<std::reference_wrapper<lib::effect<char>>, char>> state = {},
    const gen::weight &size = gen::weight::current) {
  enum command { CALL, HANDLE, CREATE };
  auto i = size;
  while (gen::next(1, i--)) {
    switch (gen::weighted<command>({{4 * !state.empty(), CALL}, {2 * !state.empty(), HANDLE}, {1, CREATE}})) {

    case CALL: {
      auto &&pair = gen::element(state);
      REQUIRE_EQ(pair.first.get()(), pair.second);
    } break;

    case HANDLE: {
      auto value = gen::arbitrary<char>();
      auto &&pair = gen::element(state);
      auto _ = pair.first.get().handle([=] {
        effect_test(state, size / 4);
        return value;
      });

      std::swap(pair.second, value);
      effect_test(state, size / 4);
      std::swap(pair.second, value);
    } break;

    case CREATE: {
      auto value = gen::arbitrary<char>();
      lib::effect<char> eff([=] {
        effect_test(state, size / 4);
        return value;
      });

      state.emplace_back(std::ref(eff), value);
      effect_test(state, size / 4);
      state.pop_back();
    } break;
    }
  }
}

TEST_CASE("lib::effect") {
  test::check([] { effect_test(); });
}

TEST_CASE("effect example") {
  lib::effect<bool> example0([] { return (throw 0, true); });
  lib::effect<bool> example1([] { return true; });

  REQUIRE_THROWS_AS(example0(), int);
  REQUIRE(example1());

  {
    auto _0 = example0.handle([&] {
      REQUIRE_THROWS_AS(example0(), int);
      return true;
    });

    auto _1 = example1.handle([&] {
      REQUIRE(example1());
      REQUIRE(example0());
      return false;
    });

    REQUIRE(example0());
    REQUIRE_FALSE(example1());
  }

  REQUIRE_THROWS_AS(example0(), int);
  REQUIRE(example1());
}
