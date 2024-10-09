#ifndef HALCHECK_LIB_FUNCTIONAL_HPP
#define HALCHECK_LIB_FUNCTIONAL_HPP

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <functional> // IWYU pragma: export
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

/// @brief See https://en.cppreference.com/w/cpp/utility/functional/invoke.
/// @ingroup util
template<typename F, typename... Args, HALCHECK_REQUIRE(std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(
    noexcept(std::mem_fn(f)(std::forward<Args>(args)...))) -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
  return std::mem_fn(f)(std::forward<Args>(args)...);
}

/// @brief See https://en.cppreference.com/w/cpp/utility/functional/invoke.
/// @ingroup util
template<typename F, typename... Args, HALCHECK_REQUIRE(!std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(noexcept(
    std::forward<F>(f)(std::forward<Args>(args)...))) -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
  return std::forward<F>(f)(std::forward<Args>(args)...);
}

/// @brief See https://en.cppreference.com/w/cpp/types/result_of.
/// @ingroup meta
template<typename F, typename... Args>
using invoke_result_t = decltype(lib::invoke(std::declval<F>(), std::declval<Args>()...));

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename R, typename F, typename... Args>
using invocable_r = lib::convertible<lib::invoke_result_t<F, Args...>, R>;

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename R, typename F, typename... Args>
struct is_invocable_r : lib::is_detected<lib::invocable_r, R, F, Args...> {};

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename F, typename... Args>
using invocable = lib::invoke_result_t<F, Args...>;

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename F, typename... Args>
struct is_invocable : lib::is_detected<lib::invocable, F, Args...> {};

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename F, typename... Args>
using nothrow_invocable = lib::enable_if_t<noexcept(lib::invoke(std::declval<F>(), std::declval<Args>()...))>;

/// @brief See https://en.cppreference.com/w/cpp/types/is_invocable.
/// @ingroup meta
template<typename F, typename... Args>
struct is_nothrow_invocable : lib::is_detected<lib::nothrow_invocable, F, Args...> {};

template<typename... Args>
class overload {};

template<typename T>
class overload<T> : private T {
public:
  using T::operator();
  explicit overload(T head) : T(std::move(head)) {}
};

template<typename T, typename... Args>
class overload<T, Args...> : private T, private overload<Args...> {
public:
  using T::operator();
  using overload<Args...>::operator();
  explicit overload(T head, Args... tail) : T(std::move(head)), overload<Args...>(std::move(tail)...) {}
};

/// @brief Constructs an overloaded functor from a set of pre-existing functors.
/// @tparam Args The types of functors to combine.
/// @param args The functors to combine.
/// @return A single value that contains all of \p args and exposes their call operators.
/// @ingroup util
template<typename... Args>
lib::overload<Args...> make_overload(Args... args) {
  return lib::overload<Args...>(std::move(args)...);
}

/// @brief A function object that calls the constructor of a given type.
///
/// @tparam T The type of value to construct.
template<typename T>
struct constructor {
  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  T operator()(Args... args) {
    return T(std::forward<Args>(args)...);
  }
};

template<typename>
class function_view;

template<typename R, typename... Args>
class function_view<R(Args...)> {
public:
  template<
      typename F,
      HALCHECK_REQUIRE(!std::is_const<F>()),
      HALCHECK_REQUIRE(!std::is_pointer<lib::decay_t<F>>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<R, lib::decay_t<F> &, Args...>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<F>, function_view>())>
  function_view(F &&func) noexcept // NOLINT: implicit reference to functor
      : _impl(closure{std::addressof(func), [](void *impl, Args... args) {
                        return lib::invoke(*reinterpret_cast<lib::decay_t<F> *>(impl), std::forward<Args>(args)...);
                      }}) {}

  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  R operator()(Args... args) const {
    return lib::visit(
        lib::make_overload(
            [&](const closure &func) { return func.invoke(func.self, std::forward<Args>(args)...); },
            [&](R (*func)(Args...)) { return func(std::forward<Args>(args)...); }),
        _impl);
  }

private:
  struct closure {
    void *self;
    R (*invoke)(void *, Args...);
  };

  lib::variant<R (*)(Args...), closure> _impl;
};

template<typename R, typename... Args>
class function_view<R(Args...) const> {
public:
  template<
      typename F,
      HALCHECK_REQUIRE(!std::is_pointer<lib::decay_t<F>>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<R, const lib::decay_t<F> &, Args...>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<F>, function_view>())>
  function_view(F &&func) noexcept // NOLINT: implicit reference to functor
      : _impl(closure{
            std::addressof(func), [](const void *impl, Args... args) {
              return lib::invoke(*reinterpret_cast<const lib::decay_t<F> *>(impl), std::forward<Args>(args)...);
            }}) {}

  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  R operator()(Args... args) const {
    return lib::visit(
        lib::make_overload(
            [&](const closure &func) { return func.invoke(func.self, std::forward<Args>(args)...); },
            [&](R (*func)(Args...)) { return func(std::forward<Args>(args)...); }),
        _impl);
  }

private:
  struct closure {
    const void *self;
    R (*invoke)(const void *, Args...);
  };

  lib::variant<R (*)(Args...), closure> _impl;
};

}} // namespace halcheck::lib

#endif
