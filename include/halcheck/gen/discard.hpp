#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/optional.hpp>
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
/// @tparam F The type of function to call.
/// @param max The maximum number of tries.
/// @param func The function to call.
/// @return The result of func.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::invoke_result_t<F> retry(std::intmax_t max, F func) {
  return gen::group([&] {
    {
      struct e {};
      std::uintmax_t i = 0;
      auto _0 = gen::discard.handle([] { return e(); });
      auto _1 = gen::size.handle([&] { return gen::size() + i; });
      while (true) {
        try {
          return gen::group(func);
        } catch (const e &) {
          if (max > 0 && i >= max)
            break;

          ++i;
        }
      }
    }
    gen::discard();
  });
}

/// @brief Executes a function. If the function calls discard, then it is
///        retried, up to 100 times.
/// @tparam F The type of function to call.
/// @param func The function to call.
/// @return The result of func.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::invoke_result_t<F> retry(F func) {
  return gen::retry(100, std::move(func));
}

/// @brief Executes a function. If the function calls discard before calling
/// next, then lib::nullopt is returned.
/// @tparam F The type of function to invoke.
/// @param func The function to invoke.
/// @return The return value of func, or lib::nullopt.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::optional<lib::invoke_result_t<F>> backtrack(F func) {
  return gen::group([&]() -> lib::optional<lib::invoke_result_t<F>> {
    struct discard {};
    bool ok = true;
    try {
      auto _0 = gen::next.handle([&](std::uintmax_t w0, std::uintmax_t w1) {
        ok = false;
        return gen::next(w0, w1);
      });
      auto _1 = gen::discard.handle([&] { return lib::raise(discard()); });
      return lib::invoke(std::move(func));
    } catch (const discard &) {
      if (ok)
        return lib::nullopt;
      else
        gen::discard();
    }
  });
}

}} // namespace halcheck::gen

#endif
