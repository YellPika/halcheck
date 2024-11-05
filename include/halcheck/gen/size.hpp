#ifndef HALCHECK_GEN_SIZE_HPP
#define HALCHECK_GEN_SIZE_HPP

/**
 * @defgroup gen-size gen/size
 * @brief Sized generation primitives.
 * @ingroup gen
 */

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>

#include <cstdint>

namespace halcheck { namespace gen {

/**
 * @brief An effect for obtaining a size parameter.
 * @ingroup gen-size
 */
struct size_effect {
  /**
   * @brief The default size is zero.
   * @return 0
   */
  std::uintmax_t fallback() const { return 0; }
};

/**
 * @brief Gets the maximum size of value that should be generated.
 * @return The result of `lib::effect::invoke<gen::size_effect>()`.
 * @ingroup gen-size
 */
inline std::uintmax_t size() { return lib::effect::invoke<gen::size_effect>(); }

/**
 * @brief Applies a multiplier to the result of gen::size.
 * @par Signature
 * @code
 *   scope-t scale(double amount);                                                  // (1)
 *
 *   template<typename F, typename... Args>
 *   lib::invoke_result_t<F, Args...> scale(double amount, F func, Args &&...args); // (2)
 * @endcode
 * @tparam F The type of function to invoke.
 * @tparam Args The types of arguments to pass.
 * @param amount The amount to scale the result of gen::size by.
 * @param func The function to invoke.
 * @param args The types of arguments to pass to @p func.
 * @return
 * 1. A value `x` such that all calls to `gen::size()` return `gen::size() * amount` while `x` is in scope.
 * 2. The result of `(gen::scale(amount), lib::invoke(func, std::forward<Args>(args)...))`.
 *
 * @ingroup gen-size
 */
static const struct {
private:
  struct handler : lib::effect::handler<handler, gen::size_effect> {
    explicit handler(double amount) : amount(amount) {}
    std::uintmax_t operator()(size_effect) final { return std::uintmax_t(double(gen::size()) * amount); }
    double amount;
  };

public:
  handler::owning_scope operator()(double amount) const { return handler(amount).handle(); }

  template<typename F, typename... Args>
  lib::invoke_result_t<F, Args...> operator()(double amount, F func, Args &&...args) const {
    return handler(amount).handle(std::move(func), std::forward<Args>(args)...);
  }
} scale;

}} // namespace halcheck::gen

#endif
