#ifndef HALCHECK_LIB_SCOPE_HPP
#define HALCHECK_LIB_SCOPE_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
class finally_t {
public:
  explicit finally_t(F func) noexcept : _func(std::move(func)) {}
  finally_t(finally_t &&other) noexcept : _func(std::move(other._func)) { other._func.reset(); }
  finally_t(const finally_t &) = delete;
  finally_t &operator=(const finally_t &) = delete;
  finally_t &operator=(finally_t &&) = delete;
  void *operator new(std::size_t) = delete;
  void *operator new[](std::size_t) = delete;

  ~finally_t() noexcept {
    if (_func)
      lib::invoke(std::move(*_func));
  }

private:
  lib::optional<F> _func;
};

/// @brief Executes a function on scope exit.
/// @tparam F the type of function to execute.
/// @param func The function to execute.
/// @return An object that executes the function upon destruction.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
finally_t<F> finally(F func) {
  return finally_t<F>(std::move(func));
}

/// @brief Represents an arbitrary destructable value.
class destructable {
public:
  destructable() = default;

  /// @brief Constructs a destructable that does nothing upon destruction.
  template<typename T, HALCHECK_REQUIRE(std::is_trivially_destructible<lib::decay_t<T>>())>
  destructable(T) {} // NOLINT: implicit conversion

  /// @brief Creates a destructable from an r-value.
  /// @tparam T The type of value to hold.
  /// @param value The value to hold.
  template<
      typename T,
      HALCHECK_REQUIRE(!std::is_trivially_destructible<lib::decay_t<T>>()),
      HALCHECK_REQUIRE(!std::is_reference<T>()),
      HALCHECK_REQUIRE(!std::is_same<T, destructable>())>
  destructable(T &&value) // NOLINT: implicit conversion
      : _impl(new derived<T>(std::forward<T>(value))) {}

  template<typename T, typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  explicit destructable(lib::in_place_type_t<T>, Args &&...args) : _impl(new derived<T>(std::forward<Args>(args)...)) {}

private:
  struct base {
    virtual ~base() = default;
  };

  template<typename T>
  struct derived : base {
    template<typename... Args>
    explicit derived(Args &&...args) : value(std::forward<Args>(args)...) {}

    T value;
  };

  std::unique_ptr<base> _impl;
};

template<typename T, typename... Args>
lib::destructable make_destructable(Args &&...args) {
  return lib::destructable(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

template<typename T>
struct exchange_finally_t {
public:
  exchange_finally_t(T &target, T old) : target(&target), old(std::move(old)) {}
  void operator()() const { *target = old; }

private:
  T *target, old;
};

template<typename T, typename U>
lib::finally_t<lib::exchange_finally_t<T>> tmp_exchange(T &value, U &&next) {
  return lib::finally(exchange_finally_t<T>(value, lib::exchange(value, next)));
}

}} // namespace halcheck::lib

#endif
