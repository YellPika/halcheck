#include "halcheck/test/random.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <functional>
#include <random>
#include <string>

using namespace halcheck;

test::strategy test::random(std::int_fast32_t seed) {
  struct handler : eff::handler<handler, gen::label_effect, gen::sample_effect> {
    explicit handler(std::mt19937_64 engine) : engine(engine) {}

    lib::destructable operator()(gen::label_effect args) override {
      auto previous = engine;
      engine.seed(engine() + std::hash<lib::atom>()(args.value));
      return std::make_pair(gen::label(args.value), lib::finally([&, previous] { engine = previous; }));
    }

    std::uintmax_t operator()(gen::sample_effect args) override {
      auto copy = engine;
      return std::uniform_int_distribution<std::uintmax_t>(0, args.max)(copy);
    }

    std::mt19937_64 engine;
  };

  std::mt19937_64 engine(seed);
  return [=](const std::function<void()> &func) mutable {
    auto _ = lib::finally(std::ref(engine));
    if (auto seed_str = test::read("seed")) {
      if (auto seed_eng = lib::of_string<std::mt19937_64>(*seed_str))
        return eff::handle(func, handler(*seed_eng));
    }

    test::write("seed", lib::to_string(engine));
    eff::handle(func, handler(engine));
  };
}
