#ifndef HALCHECK_LIB_FUNCTIONAL_INVOKE_HPP
#define HALCHECK_LIB_FUNCTIONAL_INVOKE_HPP

// IWYU pragma: private, include <halcheck/lib/functional.hpp>

#include <halcheck/lib/pp.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

/**
 * @brief An implementation of std::invoke.
 * @par Signature
 * @code
 *   template<typename F, typename... Args>
 *   lib::invoke_result_t<F, Args...> invoke(F &&func, Args &&... args)
 *     noexcept(lib::is_nothrow_invocable<F, Args...>())
 * @endcode
 * @see std::invoke
 * @ingroup lib-functional
 */
HALCHECK_INLINE_CONSTEXPR struct {
  template<typename F, typename... Args, HALCHECK_REQUIRE(std::is_member_pointer<lib::decay_t<F>>())>
  auto operator()(F &&f, Args &&...args) const noexcept(noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
      -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
    return std::mem_fn(f)(std::forward<Args>(args)...);
  }

  template<typename F, typename... Args, HALCHECK_REQUIRE(!std::is_member_pointer<lib::decay_t<F>>())>
  auto operator()(F &&f, Args &&...args) const noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
      -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
    return std::forward<F>(f)(std::forward<Args>(args)...);
  }
} invoke;

/**
 * @brief An implementation of std::invoke_result_t.
 * @see std::invoke_result_t
 * @ingroup lib-functional
 */
template<typename F, typename... Args>
using invoke_result_t = decltype(lib::invoke(std::declval<F>(), std::declval<Args>()...));

/** @private */
template<typename R, typename F, typename... Args>
using invocable_r = lib::convertible<lib::invoke_result_t<F, Args...>, R>;

/**
 * @brief An implementation of std::is_invocable_r.
 * @see std::is_invocable_r
 * @ingroup lib-functional
 */
template<typename R, typename F, typename... Args>
struct is_invocable_r : lib::is_detected<lib::invocable_r, R, F, Args...> {};

/** @private */
template<typename F, typename... Args>
using invocable = lib::invoke_result_t<F, Args...>;

/**
 * @brief An implementation of std::is_invocable.
 * @see std::is_invocable
 * @ingroup lib-functional
 */
template<typename F, typename... Args>
struct is_invocable : lib::is_detected<lib::invocable, F, Args...> {};

/** @private */
template<typename F, typename... Args>
using nothrow_invocable = lib::enable_if_t<noexcept(lib::invoke(std::declval<F>(), std::declval<Args>()...))>;

/**
 * @brief An implementation of std::is_nothrow_invocable.
 * @see std::is_nothrow_invocable
 * @ingroup lib-functional
 */
template<typename F, typename... Args>
struct is_nothrow_invocable : lib::is_detected<lib::nothrow_invocable, F, Args...> {};

/** @private */
template<typename R, typename F, typename... Args>
using nothrow_invocable_r = lib::void_t<lib::invocable_r<R, F, Args...>, lib::nothrow_invocable<F, Args...>>;

/**
 * @brief An implementation of std::is_nothrow_invocable_r.
 * @see std::is_nothrow_invocable_r
 * @ingroup lib-functional
 */
template<typename R, typename F, typename... Args>
struct is_nothrow_invocable_r : lib::is_detected<lib::nothrow_invocable_r, R, F, Args...> {};

}} // namespace halcheck::lib

#endif
