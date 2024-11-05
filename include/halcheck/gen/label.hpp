#ifndef HALCHECK_GEN_LABEL_HPP
#define HALCHECK_GEN_LABEL_HPP

/**
 * @defgroup gen-label gen/label
 * @brief Primitives for managing unique identifiers.
 * @ingroup gen
 */

#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <utility>

namespace halcheck { namespace gen {

/**
 * @brief An effect for extending unique identifiers passed to other random generation functions.
 * @ingroup gen-label
 */
struct label_effect {
  /**
   * @brief The unique identifier to prefix. Calls to another function `gen::func(my_label, ...)` should behave like
   * `gen::func(value + '/' + my_label, ...)` as long as the return value of invoking this effect remains in scope,
   * where `value + '/' + my_label` conceptually denotes the concatenation of two labels with some separator.
   */
  lib::atom value;

  /**
   * @brief By default, this effect does nothing.
   * @return A lib::finally_t<> value that does nothing upon destruction.
   */
  lib::finally_t<> fallback() const { return {}; }
};

/**
 * @brief
 * @par Signature
 * @code
 *   lib::finally_t<> label(lib::atom id);                         // (1)
 *
 *   template<typename F, typename... Args>
 *   lib::finally_t<> label(lib::atom id, F func, Args &&...args); // (2)
 * @endcode
 * @brief Extends the unique identifiers passed to other random generation functions.
 * @tparam F The type of function to invoke.
 * @tparam Args The types of arguments to pass.
 * @param amount The amount to scale the result of gen::size by.
 * @param func The function to invoke.
 * @param args The types of arguments to pass to @p func.
 * @return
 * 1. `lib::effect::invoke<label_effect>(value)`
 * 2. `(lib::label(id), lib::invoke(func, std::forward<Args>(args)...))`
 * @ingroup gen-label
 */
static struct {
  lib::finally_t<> operator()(lib::atom value) const { return lib::effect::invoke<label_effect>(value); }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(lib::atom value, F func, Args &&...args) const {
    auto _ = lib::effect::invoke<label_effect>(value);
    return lib::invoke(func, std::forward<Args>(args)...);
  }
} label;

}} // namespace halcheck::gen

#endif
