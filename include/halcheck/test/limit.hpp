#ifndef HALCHECK_TEST_LIMITED_HPP
#define HALCHECK_TEST_LIMITED_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/test/strategy.hpp>

#include <exception>
#include <stdexcept>

namespace halcheck { namespace test {

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
struct limit_t {
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    std::uintmax_t successes = 0, discarded = 0;

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
  std::uintmax_t max_success;
  std::uintmax_t discard_ratio;
};

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
constexpr limit_t<Strategy>
limit(Strategy strategy, std::uintmax_t max_success = 100, std::uintmax_t discard_ratio = 10) {
  return {std::move(strategy), max_success, discard_ratio};
}

}} // namespace halcheck::test

#endif
