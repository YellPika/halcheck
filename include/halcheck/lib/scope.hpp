#ifndef HALCHECK_LIB_SCOPE_HPP
#define HALCHECK_LIB_SCOPE_HPP

/// @file

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <type_traits>

namespace halcheck { namespace lib {

/// @brief Calls a function upon destruction.
///
/// @tparam F The type of function to call upon destruction.
/// @ingroup utility
template<
    typename F = lib::move_only_function<void() &&>,
    HALCHECK_REQUIRE(lib::is_invocable<F &&>()),
    HALCHECK_REQUIRE(std::is_nothrow_move_constructible<F>())>
class finally_t {
public:
  template<
      typename G,
      HALCHECK_REQUIRE_(lib::is_invocable<G &&>()),
      HALCHECK_REQUIRE_(std::is_nothrow_move_constructible<G>())>
  friend class finally_t;

  constexpr finally_t() = default;

  /// @brief Construct an object that calls the given function upon destruction.
  ///
  /// @param func The function to call.
  constexpr explicit finally_t(F func) noexcept(std::is_nothrow_move_constructible<F>())
      : _func(lib::in_place, std::move(func)) {}

  /// @brief Delays the invocation of a \ref finally_t "finally_t"'s underlying function by transfering the
  /// responsibility of calling the function to a new \ref finally_t.
  ///
  /// @param other The object from which to transfer the function.
  finally_t(finally_t &&other) noexcept(std::is_nothrow_move_constructible<F>()) : _func(std::move(other._func)) {
    other._func.reset();
  }

  /// @brief Delays the invocation of a \ref finally_t "finally_t"'s underlying function by transfering the
  /// responsibility of calling the function to a new \ref finally_t.
  ///
  /// @param other The object from which to transfer the function.
  template<typename G, HALCHECK_REQUIRE(std::is_convertible<G, F>())>
  finally_t(finally_t<G> &&other) noexcept(std::is_constructible<F, G>()) // NOLINT: implicit conversion
      : _func(std::move(other._func)) {
    other._func.reset();
  }

  /// @brief Delays the invocation of a \ref finally_t "finally_t"'s underlying function by transfering the
  /// responsibility of calling the function to a new \ref finally_t.
  ///
  /// @param other The object from which to transfer the function.
  template<typename G, HALCHECK_REQUIRE(!std::is_convertible<G, F>()), HALCHECK_REQUIRE(std::is_constructible<F, G>())>
  explicit finally_t(finally_t<G> &&other) noexcept(std::is_constructible<F, G>()) : _func(std::move(other._func)) {
    other._func.reset();
  }

  finally_t(const finally_t &) = delete;
  finally_t &operator=(const finally_t &) = delete;
  finally_t &operator=(finally_t &&) = delete;
  void *operator new(std::size_t) = delete;
  void *operator new[](std::size_t) = delete;

  /// @brief Invokes the underlying function.
  ~finally_t() noexcept {
    if (_func)
      lib::invoke(std::move(*_func));
  }

private:
  template<typename G>
  struct combine {
    lib::optional<F> first;
    lib::optional<G> second;

    void operator()() {
      if (second)
        lib::invoke(std::move(*second));
      if (first)
        lib::invoke(std::move(*first));
    }
  };

  template<typename G>
  finally_t<combine<G>> make_combined(finally_t<G> &&other) && {
    if (!_func && !other._func)
      return finally_t<combine<G>>();

    combine<G> func{std::move(_func), std::move(other._func)};
    _func.reset();
    other._func.reset();
    return finally_t<combine<G>>(std::move(func));
  }

  /// @brief Combines two \ref finally_t values into one, such that both underlying functions are called upon
  /// destruction of the return value.
  ///
  /// @note The underlying functions are called in the _reverse_ order of arguments provided, i.e. arguments should be
  /// provided in the same order they are created.
  /// @tparam G The other type of function to call.
  /// @param lhs The second function to call.
  /// @param rhs The first function to call.
  /// @return
  template<typename G>
  friend finally_t<combine<G>> operator+(finally_t &&lhs, finally_t<G> &&rhs) {
    return std::move(lhs).make_combined(std::move(rhs));
  }

  lib::optional<F> _func;
};

/// @brief Executes a function on scope exit.
/// @tparam F the type of function to execute.
/// @param func The function to execute.
/// @return An object that executes the function upon destruction.
/// @ingroup utility
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::finally_t<F> finally(F func) {
  return lib::finally_t<F>(std::move(func));
}

template<typename T>
struct exchange_finally_t {
public:
  template<typename U>
  exchange_finally_t(T &target, U &&next) : target(&target), old(lib::exchange(target, std::forward<U>(next))) {}
  void operator()() const { *target = old; }

private:
  T *target, old;
};

template<typename T, typename U>
lib::finally_t<lib::exchange_finally_t<T>> tmp_exchange(T &value, U &&next) {
  return lib::finally(exchange_finally_t<T>(value, std::forward<U>(next)));
}

}} // namespace halcheck::lib

#endif
