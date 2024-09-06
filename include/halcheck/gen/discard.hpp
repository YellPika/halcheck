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

static const struct retry_t : gen::labelable<retry_t> {
  using gen::labelable<retry_t>::operator();

  /// @brief Executes a function. If the function calls discard, then it is
  ///        retried, up to a certain number of times.
  /// @tparam F The type of function to call.
  /// @tparam ...Args The types of arguments to pass to the function.
  /// @param max The maximum number of tries.
  /// @param func The function to call.
  /// @param args The arguments to pass to the function.
  /// @return The result of func.
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(std::uintmax_t max, F func, Args &&...args) const {
    for (std::uintmax_t i = 0;; i++) {
      try {
        return gen::label(lib::number(i), func, std::forward<Args>(args)...);
      } catch (const gen::discard &) {
        if (max > 0 && i >= max)
          throw;
      }
    }
  }

  /// @brief Executes a function. If the function calls discard, then it is
  ///        retried, up to 100 times.
  /// @tparam F The type of function to call.
  /// @tparam ...Args The types of arguments to pass to the function.
  /// @param func The function to call.
  /// @param args The arguments to pass to the function.
  /// @return The result of func.
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(F func, Args &&...args) const {
    return (*this)(100, std::move(func), std::forward<Args>(args)...);
  }
} retry;

}} // namespace halcheck::gen

#endif
