#ifndef HALCHECK_LIB_FUNCTIONAL_HPP
#define HALCHECK_LIB_FUNCTIONAL_HPP

#include <halcheck/lib/type_traits.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace halcheck { namespace lib {

#if __cplusplus >= 201703L
using std::invoke;
using std::invoke_result_t;
using std::is_invocable;
using std::is_invocable_r;
#else
template<typename F, typename... Args, HALCHECK_REQUIRE(std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(noexcept(std::mem_fn(f)(std::forward<Args>(args)...)))
    -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
  return std::mem_fn(f)(std::forward<Args>(args)...);
}

template<typename F, typename... Args, HALCHECK_REQUIRE(!std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(noexcept(std::forward<F>(f)(std::forward<Args>(args)...)))
    -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
  return std::forward<F>(f)(std::forward<Args>(args)...);
}

template<typename F, typename... Args>
using invoke_result_t = decltype(lib::invoke(std::declval<F>(), std::declval<Args>()...));

namespace detail {

template<typename R, typename F, typename... Args>
using is_invocable_r_helper = lib::enable_if_t<std::is_convertible<lib::invoke_result_t<F, Args...>, R>{}>;

template<typename F, typename... Args>
using is_invocable_helper = lib::invoke_result_t<F, Args...>;

} // namespace detail

template<typename R, typename F, typename... Args>
struct is_invocable_r : lib::is_detected<detail::is_invocable_r_helper, R, F, Args...> {};

template<typename F, typename... Args>
struct is_invocable : lib::is_detected<detail::is_invocable_helper, F, Args...> {};
#endif

}} // namespace halcheck::lib

#endif
