#ifndef HALCHECK_TEST_RANDOM_HPP
#define HALCHECK_TEST_RANDOM_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <random>

namespace halcheck { namespace test {

/// @brief A strategy that produces random bit-sequences.
struct random {
public:
  /// @brief Creates a strategy with the given seed.
  /// @param seed The seed value driving random generation.
  /// @param max_size The maximum size of generated values.
  constexpr random(std::uint_fast32_t seed = 0, std::uintmax_t max_size = 0) : seed(seed), max_size(max_size) {}

  /// @brief Invokes the strategy.
  /// @tparam F The type of function to provide bit-sequences.
  /// @param func The function to provide bit-sequences.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    struct discard {};

    std::default_random_engine engine(seed);
    std::uintmax_t size = 0;

    auto strategy = [&](const gen::weight &w0, const gen::weight &w1) {
      auto x0 = w0(size);
      auto x1 = w1(size);
      return std::uniform_int_distribution<std::uintmax_t>(1, x0 + x1)(engine) > x0;
    };

    while (true) {
      try {
        auto _0 = gen::discard.handle([] { return discard(); });
        auto _1 = gen::next.handle(strategy);
        lib::invoke(func);
      } catch (const discard &) {
      }

      ++size;
      if (max_size > 0)
        size %= max_size;
    }
  }

private:
  std::uint_fast32_t seed;
  std::uintmax_t max_size;
};

}} // namespace halcheck::test

#endif
