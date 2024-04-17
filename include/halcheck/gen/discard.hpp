#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/raise.hpp>

namespace halcheck { namespace gen {

/// @brief Calls gen::discard if the condition is false.
/// @param cond The condition to check.
void guard(bool cond);

/// @brief Exits a test-case without indicating failure.
extern lib::effect<lib::raise> discard;

/// @brief Causes a test to succeed immediately. This function may be a no-op in
///        certain circumstances, such as during shrinking.
extern lib::effect<void> succeed;

/// @brief Executes a function. If the function calls discard, then it is
///        retried, up to a certain number of times.
/// @tparam T The type of function to call.
/// @param max The maximum number of tries.
/// @param func The function to call.
/// @return The result of func.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::invoke_result_t<F> retry(std::intmax_t max, F func) {
  return gen::group([&] {
    {
      struct e {};
      auto _ = discard.handle([]() { return e(); });
      while (true) {
        try {
          return gen::group(func);
        } catch (const e &) {
          if (max > 0 && --max == 0)
            break;
        }
      }
    }
    return static_cast<lib::invoke_result_t<F>>(gen::discard());
  });
}

/// @brief Executes a function. If the function calls discard, then it is
///        retried, up to 100 times.
/// @tparam T The type of function to call.
/// @param func The function to call.
/// @return The result of func.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::invoke_result_t<F> retry(F func) {
  return gen::retry(100, std::move(func));
}

}} // namespace halcheck::gen

#endif
