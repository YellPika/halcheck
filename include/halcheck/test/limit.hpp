#ifndef HALCHECK_TEST_LIMITED_HPP
#define HALCHECK_TEST_LIMITED_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/test/strategy.hpp>

#include <exception>
#include <stdexcept>

namespace halcheck { namespace test {

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
struct limit_t {
public:
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    std::uintmax_t successes = 0, discarded = 0;

    auto max_success = this->max_success.value_or(default_max_success());
    auto discard_ratio = this->discard_ratio.value_or(default_discard_ratio());

    lib::invoke(strategy, [&] {
      if (successes >= max_success)
        gen::succeed();

      auto _ = gen::discard.handle([&] {
        if (discard_ratio > 0 && ++discarded / discard_ratio >= max_success)
          gen::succeed();

        return gen::discard();
      });
      lib::invoke(func);
      successes++;
    });

    if (successes < max_success && discarded / discard_ratio >= max_success)
      throw std::runtime_error("discard limit reached");
  }

  Strategy strategy;
  lib::constexpr_optional<std::uintmax_t> max_success;
  lib::constexpr_optional<std::uintmax_t> discard_ratio;

private:
  static std::uintmax_t default_max_success() {
    static const char *var = std::getenv("HALCHECK_MAX_SUCCESS");
    static int value = var ? std::atoi(var) : 100;
    return value;
  }

  static std::uintmax_t default_discard_ratio() {
    static const char *var = std::getenv("HALCHECK_DISCARD_RATIO");
    static int value = var ? std::atoi(var) : 10;
    return value;
  }
};

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
constexpr limit_t<Strategy> limit(
    Strategy strategy,
    lib::constexpr_optional<std::uintmax_t> max_success = {},
    lib::constexpr_optional<std::uintmax_t> discard_ratio = {}) {
  return {std::move(strategy), max_success, discard_ratio};
}

}} // namespace halcheck::test

#endif
