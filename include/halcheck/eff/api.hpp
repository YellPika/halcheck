#ifndef HALCHECK_EFF_API_HPP
#define HALCHECK_EFF_API_HPP

#include <halcheck/eff/context.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace eff {

template<typename T>
struct is_effect : lib::is_detected<eff::result_t, T> {};

template<typename T, HALCHECK_REQUIRE(eff::is_effect<T>())>
eff::result_t<T> invoke(T args) {
  return eff::context::invoke(std::move(args));
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(eff::is_effect<T>()),
    HALCHECK_REQUIRE(lib::is_brace_constructible<T, Args...>())>
eff::result_t<T> invoke(Args &&...args) {
  return eff::context::invoke(T{std::forward<Args>(args)...});
}

template<typename T, typename... Args>
using handler = eff::context::handler<T, Args...>;

namespace detail {
template<typename T, typename... Args>
inline std::true_type is_handler_helper(const eff::context::handler<T, Args...> &);
inline std::false_type is_handler_helper(...);
} // namespace detail

template<typename T>
using is_handler = decltype(detail::is_handler_helper(std::declval<T>()));

template<
    typename F,
    typename T,
    HALCHECK_REQUIRE(lib::is_invocable<F>()),
    HALCHECK_REQUIRE(eff::is_handler<lib::decay_t<T>>())>
lib::invoke_result_t<F> handle(F func, T &&handler) {
  return eff::context::handle(std::move(func), std::forward<T>(handler));
}

template<typename F>
using wrap_t = eff::context::wrap_t<F>;

static const struct {
  template<typename F>
  eff::wrap_t<F> operator()(F func) const {
    return eff::wrap_t<F>(std::move(func));
  }
} wrap;

template<typename F, typename... Args>
lib::invoke_result_t<F, Args...> reset(F func, Args &&...args) {
  return eff::context::reset(std::move(func), std::forward<Args>(args)...);
}

}} // namespace halcheck::eff

#endif
