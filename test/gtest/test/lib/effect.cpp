#include <halcheck/ext/gtest.hpp>
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
    std::clog << "CALL " << fmt::show(pair.second) << std::endl;
    ASSERT_EQ(pair.first.get()(), pair.second);
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
    std::clog << "HANDLE " << fmt::show(pair.second) << std::endl;

    std::swap(pair.second, value);
    auto _1 = gen::size.handle(gen::size() / 4);
    effect_test(state);
    std::swap(pair.second, value);
  };

  auto create = [&] {
    auto value = gen::arbitrary<char>();
    lib::effect<char> eff([=] { return value; });
    std::clog << "CREATE " << fmt::show(value) << std::endl;

    state.emplace_back(std::ref(eff), value);
    auto _ = gen::size.handle(gen::size() / 4);
    effect_test(state);
    state.pop_back();
  };

  gen::repeat([&] {
    gen::retry(25, [&] {
      gen::weighted<std::function<void()>>({
          {4, call  },
          {2, handle},
          {1, create}
      })();
    });
  });
}

HALCHECK_TEST(lib, effect) { effect_test(); }

HALCHECK_TEST(lib, effect_example) {
  lib::effect<bool> example0([] { return (throw 0, true); });
  lib::effect<bool> example1([] { return true; });

  EXPECT_THROW(example0(), int);
  EXPECT_TRUE(example1());

  {
    auto _0 = example0.handle([&] {
      EXPECT_THROW(example0(), int);
      return true;
    });

    auto _1 = example1.handle([&] {
      EXPECT_TRUE(example1());
      EXPECT_TRUE(example0());
      return false;
    });

    EXPECT_TRUE(example0());
    EXPECT_FALSE(example1());
  }

  EXPECT_THROW(example0(), int);
  EXPECT_TRUE(example1());
}
