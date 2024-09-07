#ifndef HALCHECK_LIB_FUNCTIONAL_HPP
#define HALCHECK_LIB_FUNCTIONAL_HPP

#include <halcheck/lib/type_traits.hpp>

#include <functional> // IWYU pragma: export
#include <type_traits>

namespace halcheck { namespace lib {

#if __cplusplus >= 201703L
using std::invoke;
using std::invoke_result_t;
using std::is_invocable;
using std::is_invocable_r;
#else
template<typename F, typename... Args, HALCHECK_REQUIRE(std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(
    noexcept(std::mem_fn(f)(std::forward<Args>(args)...))) -> decltype(std::mem_fn(f)(std::forward<Args>(args)...)) {
  return std::mem_fn(f)(std::forward<Args>(args)...);
}

template<typename F, typename... Args, HALCHECK_REQUIRE(!std::is_member_pointer<lib::decay_t<F>>())>
auto invoke(F &&f, Args &&...args) noexcept(noexcept(
    std::forward<F>(f)(std::forward<Args>(args)...))) -> decltype(std::forward<F>(f)(std::forward<Args>(args)...)) {
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

template<typename...>
class overload_t {};

template<typename T>
class overload_t<T> : private T {
public:
  using T::operator();
  explicit overload_t(T head) : T(std::move(head)) {}
};

template<typename T, typename... Args>
class overload_t<T, Args...> : private T, private overload_t<Args...> {
public:
  using T::operator();
  using overload_t<Args...>::operator();
  explicit overload_t(T head, Args... tail) : T(std::move(head)), overload_t<Args...>(std::move(tail)...) {}
};

/// @brief Constructs an overloaded functor from a set of pre-existing functors.
/// @tparam Args The types of functors to combine.
/// @param args The functors to combine.
template<typename... Args>
lib::overload_t<Args...> overload(Args... args) {
  return lib::overload_t<Args...>(std::move(args)...);
}

/// @brief Enables copy and move assignment for types that only support copy and move constructors.
/// @tparam T The type of function to wrap.
template<typename T, typename = void>
class assignable {
public:
  assignable() = default;
  assignable(T impl) : impl(std::move(impl)) {} // NOLINT

  template<typename... Args>
  lib::invoke_result_t<T, Args...> operator()(Args &&...args) {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  template<typename... Args>
  lib::invoke_result_t<T, Args...> operator()(Args &&...args) const {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  operator T() const { return impl; } // NOLINT

  T &operator*() const { return impl; }

  T *operator->() const { return &impl; }

private:
  T impl;
};

template<typename T>
class assignable<
    T,
    lib::enable_if_t<
        std::is_move_constructible<T>() && !std::is_move_assignable<T>() && !std::is_copy_constructible<T>()>> {
public:
  assignable() = default;
  assignable(T impl) : impl(std::move(impl)) {} // NOLINT
  assignable(assignable &&) = default;
  assignable(const assignable &) = delete;
  assignable &operator=(const assignable &) = delete;
  ~assignable() = default;

  assignable &
  operator=(assignable &&other) noexcept(std::is_nothrow_destructible<T>() && std::is_nothrow_move_constructible<T>()) {
    if (this != &other) {
      impl.~T();
      new (&impl) T(std::move(other.impl));
    }
    return *this;
  }

  template<typename... Args>
  lib::invoke_result_t<T, Args...> operator()(Args &&...args) {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  template<typename... Args>
  lib::invoke_result_t<T, Args...> operator()(Args &&...args) const {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  operator T() const { return impl; } // NOLINT

  T &operator*() const { return impl; }

  T *operator->() const { return &impl; }

private:
  T impl;
};

template<typename T>
class assignable<T, lib::enable_if_t<std::is_copy_constructible<T>() && !std::is_copy_assignable<T>()>> {
public:
  assignable() = default;
  assignable(T impl) : impl(std::move(impl)) {} // NOLINT
  assignable(assignable &&) = default;
  assignable(const assignable &) = default;
  ~assignable() = default;

  assignable &
  operator=(assignable &&other) noexcept(std::is_nothrow_destructible<T>() && std::is_nothrow_move_constructible<T>()) {
    if (this != &other) {
      impl.~T();
      new (&impl) T(std::move(other.impl));
    }
    return *this;
  }

  assignable &operator=(const assignable &other) {
    if (this != &other) {
      impl.~T();
      new (&impl) T(other.impl);
    }
    return *this;
  }

  template<typename... Args, HALCHECK_REQUIRE(lib::is_invocable<T &, Args...>())>
  lib::invoke_result_t<T &, Args...> operator()(Args &&...args) {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  template<typename... Args, HALCHECK_REQUIRE(lib::is_invocable<const T &, Args...>())>
  lib::invoke_result_t<const T &, Args...> operator()(Args &&...args) const {
    return lib::invoke(impl, std::forward<Args>(args)...);
  }

  operator T() const { return impl; } // NOLINT

  T &operator*() const { return impl; }

  T *operator->() const { return &impl; }

private:
  T impl;
};

/// @brief Wraps a value such that it can be assigned.
/// @tparam T The type of value to wrap.
/// @param value The value to wrap.
/// @return An assignable version of value.
template<typename T>
assignable<T> make_assignable(T value) {
  return assignable<T>(std::move(value));
}

template<typename T>
struct constructor {
  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  T operator()(Args... args) {
    return T(std::forward<Args>(args)...);
  }
};

}} // namespace halcheck::lib

#endif
