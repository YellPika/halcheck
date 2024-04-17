#ifndef HALCHECK_TEST_RANDOM_HPP
#define HALCHECK_TEST_RANDOM_HPP

#include <halcheck/fmt/log.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <random>

namespace halcheck { namespace test {

/// @brief A strategy that produces random bit-sequences.
struct random {
public:
  struct exit_exception {};

  /// @brief Creates a strategy with the given seed.
  /// @param seed The seed value driving random generation.
  /// @param max_size The maximum size of generated values.
  constexpr random(
      lib::constexpr_optional<std::uint_fast32_t> seed = {}, lib::constexpr_optional<std::uintmax_t> max_size = {})
      : seed(seed), max_size(max_size) {}

  /// @brief Invokes the strategy.
  /// @tparam F The type of function to provide bit-sequences.
  /// @param func The function to provide bit-sequences.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    struct discard {};
    struct succeed {};

    std::default_random_engine engine(seed_var().value_or(seed.value_or(0)));
    std::uintmax_t i = 0, size = 0, max_size = max_size_var().value_or(this->max_size.value_or(100));

    while (true) {
      fmt::log(fmt::test_case_start{i});
      auto _ = lib::finally([&] { fmt::log(fmt::test_case_end{i++}); });

      try {
        auto _0 = gen::size.handle([&] { return size; });
        auto _1 = gen::succeed.handle([] { throw succeed(); });
        auto _2 = gen::discard.handle([] { return discard(); });
        auto _3 = gen::next.handle([&](std::uintmax_t w0, std::uintmax_t w1) {
          return std::uniform_int_distribution<std::uintmax_t>(1, w0 + w1)(engine) > w0;
        });
        lib::invoke(func);
      } catch (const succeed &) {
        break;
      } catch (const discard &) {
      }

      ++size;
      if (max_size > 0)
        size %= max_size;
    }
  }

private:
  static lib::optional<std::uint_fast32_t> seed_var() {
    static const char *var = std::getenv("HALCHECK_SEED");

    if (var) {
      try {
        return std::strtoumax(var, nullptr, 10);
      } catch (const std::invalid_argument &) {
      }
    }

    return {};
  }

  static lib::optional<std::uintmax_t> max_size_var() {
    static const char *var = std::getenv("HALCHECK_MAX_SIZE");

    if (var) {
      try {
        return std::strtoumax(var, nullptr, 10);
      } catch (const std::invalid_argument &) {
      }
    }

    return {};
  }

  lib::constexpr_optional<std::uint_fast32_t> seed;
  lib::constexpr_optional<std::uintmax_t> max_size;
};

}} // namespace halcheck::test

#endif
