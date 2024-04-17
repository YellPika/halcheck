#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/element.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <functional>
#include <vector>

using namespace halcheck;

using model = std::vector<std::pair<std::reference_wrapper<lib::effect<char>>, char>>;

static void effect_test(model state = {}) {
  auto call = [&] {
    gen::guard(!state.empty());
    auto &&pair = gen::element(state);
    CHECK_EQ(pair.first.get()(), pair.second);
  };

  auto handle = [&] {
    gen::guard(!state.empty());
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
  };

  auto create = [&] {
    auto value = gen::arbitrary<char>();
    lib::effect<char> eff([=] { return value; });

    state.emplace_back(std::ref(eff), value);
    auto _ = gen::size.handle(gen::size() / 4);
    effect_test(state);
    state.pop_back();
  };

  gen::repeat([&] {
    gen::retry([&] {
      gen::weighted<std::function<void()>>({
          {4, call  },
          {2, handle},
          {1, create}
      })();
    });
  });
}

HALCHECK_TEST_CASE("lib::effect") { effect_test(); }

TEST_CASE("effect example") {
  lib::effect<int> example0([] { return (throw std::runtime_error("OH NO"), 0); });
  lib::effect<int> example1([] { return 0; });

  CHECK_THROWS_AS(example0(), std::runtime_error);
  CHECK_EQ(example1(), 0);

  {
    auto _0 = example0.handle([&] {
      CHECK_THROWS_AS(example0(), std::runtime_error);
      return 1;
    });

    auto _1 = example1.handle([&] {
      CHECK_EQ(example0(), 1);
      CHECK_EQ(example1(), 0);

      auto _2 = example0.handle([&] {
        CHECK_EQ(example0(), 1);
        CHECK_EQ(example1(), 0);
        return 2;
      });

      auto _3 = example1.handle([&] {
        CHECK_EQ(example0(), 2);
        CHECK_EQ(example1(), 0);
        return 3;
      });

      CHECK_EQ(example0(), 2);
      CHECK_EQ(example1(), 3);

      return 1;
    });

    CHECK_EQ(example0(), 1);
    CHECK_EQ(example1(), 1);
  }

  CHECK_THROWS_AS(example0(), std::runtime_error);
  CHECK_EQ(example1(), 0);
}
