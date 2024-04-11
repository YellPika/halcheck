#ifndef HALCHECK_TEST_RANDOM_HPP
#define HALCHECK_TEST_RANDOM_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

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

    std::default_random_engine engine(seed.value_or(default_seed()));
    std::uintmax_t size = 0, max_size = this->max_size.value_or(default_max_size());

    while (true) {
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
  static std::uint_fast32_t default_seed() {
    static const char *var = std::getenv("HALCHECK_SEED");
    static int value = var ? std::atoi(var) : 0;
    return value;
  }

  static std::uintmax_t default_max_size() {
    static const char *var = std::getenv("HALCHECK_MAX_SIZE");
    static int value = var ? std::atoi(var) : 0;
    return value;
  }

  lib::constexpr_optional<std::uint_fast32_t> seed;
  lib::constexpr_optional<std::uintmax_t> max_size;
};

}} // namespace halcheck::test

#endif
