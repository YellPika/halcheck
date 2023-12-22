#ifndef HALCHECK_TEST_LIMITED_HPP
#define HALCHECK_TEST_LIMITED_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstddef>
#include <exception>

namespace halcheck { namespace test {

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
struct limited_t {
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    struct finish {};
    struct discard : std::exception {
      const char *what() const noexcept override { return "discard limit reached"; }
    };

    std::uintmax_t successes = 0, discarded = 0;
    bool shrinking = false;

    try {
      strategy([&] {
        if (successes >= max_success)
          throw finish();

        try {
          auto handler = gen::discard.handle([] { return discard(); });
          lib::invoke(func);
          successes++;
        } catch (const discard &) {
          if (!shrinking && discard_ratio > 0 && ++discarded / discard_ratio >= max_success)
            throw;
        } catch (...) {
          shrinking = true;
          throw;
        }
      });
    } catch (const finish &) {
    }
  }

  Strategy strategy;
  std::uintmax_t max_success;
  std::uintmax_t discard_ratio;
};

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
constexpr limited_t<Strategy>
limited(Strategy strategy, std::uintmax_t max_success = 100, std::uintmax_t discard_ratio = 10) {
  return {std::move(strategy), max_success, discard_ratio};
}

}} // namespace halcheck::test

#endif
