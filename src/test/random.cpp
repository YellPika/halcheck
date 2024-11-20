#include "halcheck/test/random.hpp"

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <random>

using namespace halcheck;

namespace {
struct succeed_exception : gen::result_exception {
  const char *what() const noexcept override { return "random.cpp:succeed_exception"; }
};

struct handler
    : lib::effect::handler<handler, gen::label_effect, gen::sample_effect, gen::size_effect, gen::succeed_effect> {
  explicit handler(std::mt19937_64 engine, std::uintmax_t size) : engine(engine), size(size) {}

  lib::finally_t<> operator()(gen::label_effect args) final {
    auto previous = engine;
    engine.seed(engine() + std::hash<lib::atom>()(args.value));
    return gen::label(args.value) + lib::finally([&, previous] { engine = previous; });
  }

  std::uintmax_t operator()(gen::sample_effect args) final {
    auto copy = engine;
    return std::uniform_int_distribution<std::uintmax_t>(0, args.max)(copy);
  }

  std::uintmax_t operator()(gen::size_effect) final { return size; }

  void operator()(gen::succeed_effect) final { throw succeed_exception(); }

  std::mt19937_64 engine;
  std::uintmax_t size;
};
} // namespace

test::strategy test::random() {
  return [](lib::function_view<void()> func) {
    auto engine = test::read<std::mt19937_64>("SEED").value_or(std::mt19937_64()); // NOLINT: need predictable value
    auto max_success = test::read<std::uintmax_t>("MAX_SUCCESS").value_or(100);
    auto max_size = test::read<std::uintmax_t>("MAX_SIZE").value_or(100);
    auto discard_ratio = test::read<std::uintmax_t>("DISCARD_RATIO").value_or(10);
    auto size = test::read<std::uintmax_t>("SIZE").value_or(0);

    test::write("MAX_SUCCESS", 1);

    std::uintmax_t successes = 0, discarded = 0;
    while (max_success == 0 || successes < max_success) {
      test::write("SEED", engine);
      test::write("SIZE", size);

      try {
        handler(engine, size).handle(func);
        ++successes;
      } catch (const gen::discard_exception &) {
        if (max_success > 0 && discard_ratio > 0 && ++discarded / discard_ratio >= max_success)
          throw test::discard_limit_exception();
      } catch (const succeed_exception &) {
        return;
      }

      ++size;
      if (max_size != 0)
        size %= max_size;

      lib::ignore = engine();
    }
  };
}
