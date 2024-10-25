#ifndef HALCHECK_LIB_FUNCTIONAL_HPP
#define HALCHECK_LIB_FUNCTIONAL_HPP

/// @file
/// @brief Utilities for functional programming.
/// @see https://en.cppreference.com/w/cpp/header/functional

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <functional> // IWYU pragma: export
#include <initializer_list>
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

/// @brief An implementation of std::invoke.
/// @see std::invoke
static const struct {
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

/// @brief An implementation of std::invoke_result_t
/// @see std::invoke_result_t
template<typename F, typename... Args>
using invoke_result_t = decltype(lib::invoke(std::declval<F>(), std::declval<Args>()...));

/// @private
template<typename R, typename F, typename... Args>
using invocable_r = lib::convertible<lib::invoke_result_t<F, Args...>, R>;

/// @brief An implementation of std::is_invocable_r
/// @see std::is_invocable_r
template<typename R, typename F, typename... Args>
struct is_invocable_r : lib::is_detected<lib::invocable_r, R, F, Args...> {};

/// @private
template<typename F, typename... Args>
using invocable = lib::invoke_result_t<F, Args...>;

/// @brief An implementation of std::is_invocable
/// @see std::is_invocable
template<typename F, typename... Args>
struct is_invocable : lib::is_detected<lib::invocable, F, Args...> {};

/// @private
template<typename F, typename... Args>
using nothrow_invocable = lib::enable_if_t<noexcept(lib::invoke(std::declval<F>(), std::declval<Args>()...))>;

/// @brief An implementation of std::is_nothrow_invocable
/// @see std::is_nothrow_invocable
template<typename F, typename... Args>
struct is_nothrow_invocable : lib::is_detected<lib::nothrow_invocable, F, Args...> {};

/// @private
template<typename R, typename F, typename... Args>
using nothrow_invocable_r = lib::void_t<lib::invocable_r<R, F, Args...>, lib::nothrow_invocable<F, Args...>>;

/// @brief An implementation of std::is_nothrow_invocable_r
/// @see std::is_nothrow_invocable_r
template<typename R, typename F, typename... Args>
struct is_nothrow_invocable_r : lib::is_detected<lib::nothrow_invocable_r, R, F, Args...> {};

/// @brief Represents an overload set as a single value.
///
/// @tparam Args The remaining overloads to represent.
template<typename... Args>
struct overload {
#ifdef HALCHECK_DOXYGEN
  /// @brief Construct a new overload set.
  /// @param args The set of overloads to represent.
  /// @post <tt> overload(fs...)(xs...) = f(xs...) </tt>, where \c f is the unique element of \c fs such that
  /// <tt> @ref "lib::is_invocable"<decltype(f), decltype(xs)...>() </tt> is satisfied.
  explicit overload(Args... args);
#endif
};

template<typename T>
struct overload<T> : private T {
  using T::operator();
  explicit overload(T head) : T(std::move(head)) {}
};

template<typename T, typename... Args>
struct overload<T, Args...> : private T, private overload<Args...> {
  using T::operator();
  using overload<Args...>::operator();
  explicit overload(T head, Args... tail) : T(std::move(head)), overload<Args...>(std::move(tail)...) {}
};

/// @brief Constructs an overloaded functor from a set of pre-existing functors.
/// @tparam Args The types of functors to combine.
/// @param args The functors to combine.
/// @return <tt> @ref "lib::overload"<Args...>(@ref "std::move"(args)...) </tt>.
template<typename... Args>
lib::overload<Args...> make_overload(Args... args) {
  return lib::overload<Args...>(std::move(args)...);
}

/// @brief A function object that calls the constructor of a given type.
/// @tparam T The type of value to construct.
template<typename T>
struct constructor {
  /// @brief Invokes a constructor of @p T.
  /// @tparam Args The type of arguments to pass to @p T::T. Must satisfy <tt> @ref "std::is_constructible"<T,
  /// Args...>() </tt>.
  /// @param args The arguments to pass to @p T::T.
  /// @return The constructed object.
  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  T operator()(Args... args) {
    return T(std::forward<Args>(args)...);
  }
};

template<typename T>
class function_view;

/// @brief Represents a reference to a function object.
///
/// @tparam R The return type of the function to refer to.
/// @tparam Args The argument types of the function to refer to.
template<typename R, typename... Args>
class function_view<R(Args...)> {
public:
  /// @brief Constructs a new function view from a function object.
  /// @tparam F The type of function to refer to.
  /// @param func The function to refer to.
  /// @post <tt> function_view(f)(xs...) == @ref "lib::invoke"(f, xs...) </tt>
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

  /// @brief Constructs a new function view from a function pointer.
  /// @param func The function to refer to.
  /// @post <tt> function_view(f)(xs...) == f(xs...) </tt>
  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  /// @brief Calls the underlying function.
  /// @param args The arguments to pass to the function.
  /// @return The result of calling the underlying function with the given arguments.
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

/// @brief Represents a const reference to a function object.
///
/// @tparam R The return type of the function to refer to.
/// @tparam Args The argument types of the function to refer to.
template<typename R, typename... Args>
class function_view<R(Args...) const> {
public:
  /// @brief Constructs a new function view from a function object.
  /// @tparam F The type of function to refer to.
  /// @param func The function to refer to.
  /// @post function_view(f)(xs...) == lib::invoke(f, xs...)
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

  /// @brief Constructs a new function view from a function pointer.
  /// @param func The function to refer to.
  /// @post function_view(f)(xs...) == f(xs...)
  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  /// @brief Calls the underlying function.
  /// @param args The arguments to pass to the function.
  /// @return The result of calling the underlying function with the given arguments.
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

template<typename T>
class move_only_function;

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...)> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, F, Args...>() && lib::is_invocable_r<R, F &, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) { return (*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...) const> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, const F, Args...>() && lib::is_invocable_r<R, const F &, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) const { return (*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) const = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) const override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...) const &> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, const F &, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) const { return (*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) const & = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) const & override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...) &> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, F &, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) & { return (*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) & = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) & override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...) const &&> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, const F, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) const && { return std::move(*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) const & = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) const && override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

/// @brief An implementation of std::move_only_function.
/// @see std::move_only_function
template<typename R, typename... Args>
class move_only_function<R(Args...) &&> {
public:
  template<typename F>
  static constexpr bool is_callable() {
    return lib::is_invocable_r<R, F, Args...>();
  }

  move_only_function() = default;

  constexpr move_only_function(std::nullptr_t) noexcept // NOLINT
      : _impl(nullptr) {}

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function(F &&f) // NOLINT: implicit conversion
      : move_only_function(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(f)) {}

  template<
      typename F,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, Args2... args)
      : _impl(new derived<F>(std::forward<Args2>(args)...)) {}

  template<
      typename F,
      typename T,
      typename... Args2,
      HALCHECK_REQUIRE(is_callable<F>()),
      HALCHECK_REQUIRE(std::is_constructible<F, std::initializer_list<T> &, Args2...>())>
  explicit move_only_function(lib::in_place_type_t<F>, std::initializer_list<T> il, Args2... args)
      : _impl(new derived<F>(il, std::forward<F>(args)...)) {}

  move_only_function &operator=(std::nullptr_t) noexcept { _impl.reset(); }

  template<typename F, HALCHECK_REQUIRE(is_callable<lib::decay_t<F>>())>
  move_only_function &operator=(F &&func) noexcept {
    _impl.reset(new derived<lib::decay_t<F>>(std::forward<F>(func)));
  }

  R operator()(Args... args) && { return std::move(*_impl)(std::forward<Args>(args)...); }

  void swap(move_only_function &other) noexcept { std::swap(_impl, other._impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args...) && = 0;
  };

  template<typename F>
  struct derived : base {
    template<typename... Args2>
    explicit derived(Args2 &&...args) : func(std::forward<Args2>(args)...) {}
    R operator()(Args... args) && override { return lib::invoke(std::move(func), std::forward<Args>(args)...); }
    F func;
  };

  std::unique_ptr<base> _impl;
};

}} // namespace halcheck::lib

#endif
