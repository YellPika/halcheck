#include "halcheck/test/random.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <string>

using namespace halcheck;

namespace {
struct handler : eff::handler<handler, gen::label_effect, gen::sample_effect, gen::size_effect> {
  explicit handler(std::mt19937_64 engine, std::uintmax_t size) : engine(engine), size(size) {}

  lib::destructable operator()(gen::label_effect args) override {
    auto previous = engine;
    engine.seed(engine() + std::hash<lib::atom>()(args.value));
    return std::make_pair(gen::label(args.value), lib::finally([&, previous] { engine = previous; }));
  }

  std::uintmax_t operator()(gen::sample_effect args) override {
    auto copy = engine;
    return std::uniform_int_distribution<std::uintmax_t>(0, args.max)(copy);
  }

  std::uintmax_t operator()(gen::size_effect) override { return size; }

  std::mt19937_64 engine;
  std::uintmax_t size;
};
} // namespace

test::strategy test::random() {
  return [](const std::function<void()> &func) {
    auto engine = test::read<std::mt19937_64>("SEED").value_or(std::mt19937_64());
    auto max_success = test::read<std::uintmax_t>("MAX_SUCCESS").value_or(100);
    auto max_size = test::read<std::uintmax_t>("MAX_SIZE").value_or(100);
    auto discard_ratio = test::read<std::uintmax_t>("DISCARD_RATIO").value_or(10);
    auto size = test::read<std::uintmax_t>("SIZE").value_or(0);

    test::write("MAX_SUCCESS", "1");

    std::uintmax_t successes = 0, discarded = 0;
    while (max_success == 0 || successes < max_success) {
      test::write("SEED", lib::to_string(engine));
      test::write("SIZE", lib::to_string(size));

      try {
        eff::handle(func, handler(engine, size));
        ++successes;
      } catch (const gen::discard &) {
        if (discard_ratio > 0 && ++discarded / discard_ratio >= max_success)
          throw test::discard_limit_exception();
      } catch (const gen::succeed &) {
        return;
      }

      ++size;
      if (max_size != 0)
        size %= max_size;

      engine();
    }
  };
}
