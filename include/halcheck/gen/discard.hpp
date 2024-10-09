#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <exception>

namespace halcheck { namespace gen {

/// @brief Calls gen::discard if the condition is false.
/// @param cond The condition to check.
void guard(bool cond);

struct result : std::exception {};

struct discard : gen::result {
  const char *what() const noexcept override { return "halcheck::gen::discard"; }
};

struct succeed : gen::result {
  const char *what() const noexcept override { return "halcheck::gen::succeed"; }
};

/// @brief Executes a function. If the function calls discard, then it is
///        retried, up to a certain number of times.
/// @tparam F The type of function to call.
/// @param max The maximum number of tries.
/// @param func The function to call.
/// @return The result of func.
template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
lib::invoke_result_t<F, lib::atom> retry(lib::atom id, std::uintmax_t max, F func) {
  auto _ = gen::label(id);
  for (std::uintmax_t i = 0;; i++) {
    try {
      return lib::invoke(func, i);
    } catch (const gen::discard &) {
      if (max > 0 && i >= max)
        throw;
    }
  }
}

/// @brief Executes a function. If the function calls discard, then it is
///        retried, up to 100 times.
/// @tparam F The type of function to call.
/// @param func The function to call.
/// @return The result of func.
template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
lib::invoke_result_t<F, lib::atom> retry(lib::atom id, F func) {
  return gen::retry(id, 100, std::move(func));
}

}} // namespace halcheck::gen

#endif
