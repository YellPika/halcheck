#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

/**
 * @defgroup gen-discard gen/discard
 * @brief Non-standard test results and related functions.
 * @ingroup gen
 */

#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <exception>

namespace halcheck { namespace gen {

/**
 * @brief Throws a gen::discard_exception if the given condition is `false`.
 * @param cond The condition to check.
 * @ingroup gen-discard
 */
void guard(bool cond);

/**
 * @brief A special exception type used to indicate an abnormal but non-failing result.
 * @details Test strategies should not treat the presence of such exceptions as an error.
 * @ingroup gen-discard
 */
struct result_exception : std::exception {};

/**
 * @brief An exception type thrown to indicate that the current test case is invalid.
 * @ingroup gen-discard
 */
struct discard_exception : gen::result_exception {
  const char *what() const noexcept override { return "halcheck::gen::discard_exception"; }
};

/**
 * @brief An effect for indicating that no more test cases should be generated for the current test.
 * @ingroup gen-discard
 */
struct succeed_effect {
  /**
   * @brief This effect does nothing by default.
   */
  void fallback() const {}
};

/**
 * @brief Indicates that no more test cases should be generated for the current test.
 * @ingroup gen-discard
 */
inline void succeed() { lib::effect::invoke<succeed_effect>(); }

/**
 * @brief Executes a function. If the function calls discard, then it is
 *        retried, up to a certain number of times.
 * @tparam F The type of function to call.
 * @param id A unique identifier for the generated value.
 * @param max The maximum number of tries.
 * @param func The function to call.
 * @return The result of func.
 * @ingroup gen-discard
 */
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
lib::invoke_result_t<F, lib::atom> retry(lib::atom id, std::uintmax_t max, F func) {
  auto _ = gen::label(id);
  for (std::uintmax_t i = 0;; i++) {
    try {
      return lib::invoke(func, i);
    } catch (const gen::discard_exception &) {
      if (max > 0 && i >= max)
        throw;
    }
  }
}

/**
 * @brief Executes a function. If the function calls discard, then it is
 *        retried, up to 100 times.
 * @tparam F The type of function to call.
 * @param id A unique identifier for the generated value.
 * @param func The function to call.
 * @return The result of func.
 * @ingroup gen-discard
 */
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
lib::invoke_result_t<F, lib::atom> retry(lib::atom id, F func) {
  return gen::retry(id, 100, std::move(func));
}

}} // namespace halcheck::gen

#endif
