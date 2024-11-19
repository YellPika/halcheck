#ifndef HALCHECK_LIB_FUNCTIONAL_MOVE_ONLY_FUNCTION_HPP
#define HALCHECK_LIB_FUNCTIONAL_MOVE_ONLY_FUNCTION_HPP

#include <halcheck/lib/functional/invoke.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>

namespace halcheck { namespace lib {

template<typename T>
class move_only_function;

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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

/**
 * @brief An implementation of std::move_only_function.
 * @see std::move_only_function
 * @ingroup lib-functional
 */
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
