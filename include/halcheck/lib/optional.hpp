#ifndef HALCHECK_LIB_OPTIONAL_HPP
#define HALCHECK_LIB_OPTIONAL_HPP

/// @file
/// @brief An implementation of std::optional.
/// @see https://en.cppreference.com/w/cpp/header/optional

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>

#include <cstddef>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#if __cplusplus >= 201606L
#include <optional> // IWYU pragma: export
#endif

namespace halcheck { namespace lib {

/// @brief An implementation of std::nullopt_t.
/// @see std::nullopt_t
struct nullopt_t {
  struct tag {};
  explicit constexpr nullopt_t(tag) {}
};

/// @brief An implementation of std::nullopt.
/// @see std::nullopt
static constexpr nullopt_t nullopt{nullopt_t::tag{}};

/// @brief An implementation of std::bad_optional_access.
/// @see std::bad_optional_access
struct bad_optional_access : std::exception {
  bad_optional_access() = default;
  const char *what() const noexcept override { return "halcheck::lib::bad_optional_access"; }
};

template<typename>
class optional;

namespace detail {

template<typename T, typename = void>
struct optional_base {
  constexpr optional_base() noexcept : _dummy(), _has_value(false) {}

  template<typename... Args>
  explicit constexpr optional_base(lib::in_place_t, Args &&...args)
      : _value(std::forward<Args>(args)...), _has_value(true) {}

  struct dummy {};
  union {
    dummy _dummy;
    T _value;
  };
  bool _has_value;
};

template<typename T>
struct optional_base<T &> {
  constexpr optional_base() noexcept : _value(nullptr) {}

  explicit constexpr optional_base(lib::in_place_t, T &value) noexcept : _value(std::addressof(value)) {}

  struct dummy {};
  T *_value;
};

template<typename T>
struct optional_base<T, lib::enable_if_t<!std::is_trivially_destructible<T>()>> { // NOLINT
  constexpr optional_base() noexcept : _dummy(), _has_value(false) {}

  template<typename... Args>
  explicit constexpr optional_base(lib::in_place_t, Args &&...args)
      : _value(std::forward<Args>(args)...), _has_value(true) {}

  ~optional_base() {
    if (_has_value)
      _value.~T();
  }

  struct dummy {};

  union {
    dummy _dummy;
    T _value;
  };
  bool _has_value;
};

template<typename T>
struct optional_ops : private optional_base<T> {
  using optional_base<T>::optional_base;

  using value_type = T;
  using iterator = T *;
  using const_iterator = const T *;

  iterator begin() { return std::addressof(this->_value); }
  const_iterator begin() const { return std::addressof(this->_value); }
  iterator end() { return begin() + (this->_has_value ? 1 : 0); }
  const_iterator end() const { return begin() + (this->_has_value ? 1 : 0); }

  explicit operator bool() const noexcept { return this->_has_value; }

  const T &operator*() const & { return this->_value; }
  T &operator*() & { return this->_value; }
  const T &&operator*() const && { return this->_value; }
  T &&operator*() && { return this->_value; }

  void reset() {
    if (this->_has_value) {
      this->_value.~T();
      this->_has_value = false;
    }
  }

  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  T &emplace(Args &&...args) {
    reset();
    new (std::addressof(this->_value)) T(std::forward<Args>(args)...);
    this->_has_value = true;
    return this->_value;
  }
};

template<typename T>
struct optional_ops<T &> : private optional_base<T &> {
  using optional_base<T &>::optional_base;

  using value_type = T;
  using iterator = T *;
  using const_iterator = const T *;

  iterator begin() { return this->_value; }
  const_iterator begin() const { return this->_value; }
  iterator end() { return begin() + (this->_value ? 1 : 0); }
  const_iterator end() const { return begin() + (this->_value ? 1 : 0); }

  optional_ops() = default;

  explicit operator bool() const noexcept { return this->_value; }

  const T &operator*() const { return *this->_value; }
  T &operator*() { return *this->_value; }

  void reset() { this->_value = nullptr; }

  template<typename U = T, HALCHECK_REQUIRE(std::is_convertible<T *, U *>())>
  T &emplace(U &value) {
    this->_value = std::addressof(value);
    return *this->_value;
  }
};

template<typename T, typename = void>
struct optional_copy_constructor_base : public optional_ops<T> {
  using optional_ops<T>::optional_ops;
};

template<typename T>
struct optional_copy_constructor_base<
    T,
    lib::enable_if_t<!std::is_trivially_copy_constructible<T>() && std::is_copy_constructible<T>()>>
    : public optional_ops<T> { // NOLINT
  using optional_ops<T>::optional_ops;

  optional_copy_constructor_base() = default;
  optional_copy_constructor_base &operator=(const optional_copy_constructor_base &) = default;
  optional_copy_constructor_base &operator=(optional_copy_constructor_base &&) = default;
  optional_copy_constructor_base(optional_copy_constructor_base &&) = default;
  ~optional_copy_constructor_base() = default;

  optional_copy_constructor_base(const optional_copy_constructor_base &other) {
    if (other)
      this->emplace(*other);
  }
};

template<typename T, typename = void>
struct optional_copy_assignment_base : public optional_copy_constructor_base<T> {
  using optional_copy_constructor_base<T>::optional_copy_constructor_base;
};

template<typename T>
struct optional_copy_assignment_base<
    T,
    lib::enable_if_t<
        !std::is_trivially_copy_assignable<T>() && std::is_copy_constructible<T>() && std::is_copy_assignable<T>()>>
    : public optional_copy_constructor_base<T> {
  using optional_copy_constructor_base<T>::optional_copy_constructor_base;

  optional_copy_assignment_base() = default;
  optional_copy_assignment_base(optional_copy_assignment_base &&) = default;
  optional_copy_assignment_base &operator=(optional_copy_assignment_base &&) = default;
  optional_copy_assignment_base(const optional_copy_assignment_base &) = default;
  ~optional_copy_assignment_base() = default;

  optional_copy_assignment_base &operator=(const optional_copy_assignment_base &other) {
    if (this != &other) {
      if (!other)
        this->reset();
      else if (*this)
        **this = *other;
      else
        this->emplace(*other);
    }

    return *this;
  }
};

template<typename T>
struct optional_copy_assignment_base<
    T,
    lib::enable_if_t<
        !std::is_trivially_copy_assignable<T>() && std::is_copy_constructible<T>() && !std::is_copy_assignable<T>()>>
    : public optional_copy_constructor_base<T> {
  using optional_copy_constructor_base<T>::optional_copy_constructor_base;

  optional_copy_assignment_base() = default;
  optional_copy_assignment_base(optional_copy_assignment_base &&) = default;
  optional_copy_assignment_base &operator=(optional_copy_assignment_base &&) = default;
  optional_copy_assignment_base(const optional_copy_assignment_base &) = default;
  ~optional_copy_assignment_base() = default;

  optional_copy_assignment_base &operator=(const optional_copy_assignment_base &other) {
    if (this != &other && *other)
      this->emplace(*other);

    return *this;
  }
};

template<typename T, typename = void>
struct optional_move_constructor_base : public optional_copy_assignment_base<T> {
  using optional_copy_assignment_base<T>::optional_copy_assignment_base;
};

template<typename T>
struct optional_move_constructor_base<
    T,
    lib::enable_if_t<!std::is_trivially_move_constructible<T>() && std::is_move_constructible<T>()>>
    : public optional_copy_assignment_base<T> { // NOLINT
  using optional_copy_assignment_base<T>::optional_copy_assignment_base;

  optional_move_constructor_base() = default;
  optional_move_constructor_base &operator=(const optional_move_constructor_base &) = default;
  optional_move_constructor_base &operator=(optional_move_constructor_base &&) = default;
  optional_move_constructor_base(const optional_move_constructor_base &) = default;
  ~optional_move_constructor_base() = default;

  optional_move_constructor_base(optional_move_constructor_base &&other) noexcept(
      std::is_nothrow_move_constructible<T>()) {
    if (other)
      this->emplace(std::move(*other));
  }
};

template<typename T, typename = void>
struct optional_move_assignment_base : public optional_move_constructor_base<T> {
  using optional_move_constructor_base<T>::optional_move_constructor_base;
};

template<typename T>
struct optional_move_assignment_base<
    T,
    lib::enable_if_t<!std::is_trivially_move_assignable<T>() && std::is_move_assignable<T>()>>
    : public optional_move_constructor_base<T> { // NOLINT
  using optional_move_constructor_base<T>::optional_move_constructor_base;

  optional_move_assignment_base() = default;
  optional_move_assignment_base(const optional_move_assignment_base &) = default;
  optional_move_assignment_base &operator=(const optional_move_assignment_base &) = default;
  optional_move_assignment_base(optional_move_assignment_base &&) = default;
  ~optional_move_assignment_base() = default;

  optional_move_assignment_base &operator=(optional_move_assignment_base &&other) noexcept(
      std::is_nothrow_move_constructible<T>() && std::is_nothrow_move_assignable<T>()) {
    if (this != &other) {
      if (!other)
        this->reset();
      else if (*this)
        **this = std::move(*other);
      else
        this->emplace(std::move(*other));
    }

    return *this;
  }
};

template<typename T>
struct optional_move_assignment_base<
    T,
    lib::enable_if_t<!std::is_trivially_move_assignable<T>() && !std::is_move_assignable<T>()>>
    : public optional_move_constructor_base<T> { // NOLINT
  using optional_move_constructor_base<T>::optional_move_constructor_base;

  optional_move_assignment_base() = default;
  optional_move_assignment_base(const optional_move_assignment_base &) = default;
  optional_move_assignment_base &operator=(const optional_move_assignment_base &) = default;
  optional_move_assignment_base(optional_move_assignment_base &&) = default;
  ~optional_move_assignment_base() = default;

  optional_move_assignment_base &
  operator=(optional_move_assignment_base &&other) noexcept(std::is_nothrow_move_constructible<T>()) {
    if (this != &other && *other)
      this->emplace(std::move(*other));

    return *this;
  }
};

} // namespace detail

/// @brief An implementation of std::optional.
/// @see std::optional
template<typename T>
class optional : private detail::optional_move_assignment_base<T> {
private:
  using base = detail::optional_move_assignment_base<T>;

public:
  optional() = default;

  constexpr optional(lib::nullopt_t) noexcept {} // NOLINT

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_constructible<T, const U &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(std::is_convertible<const U &, T>())>
  optional(const optional<U> &other) { // NOLINT
    if (other)
      emplace(*other);
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_constructible<T, const U &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const U &, T>())>
  explicit optional(const optional<U> &other) { // NOLINT
    if (other)
      emplace(*other);
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>())>
  optional(optional<U> &&other) { // NOLINT
    if (other)
      emplace(std::move(*other));
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const lib::optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<const lib::optional<U> &&, T>()),
      HALCHECK_REQUIRE(!std::is_convertible<U &&, T>())>
  explicit optional(optional<U> &&other) { // NOLINT
    if (other)
      emplace(std::move(*other));
  }

  template<typename U = T, typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  constexpr explicit optional(lib::in_place_t, Args &&...args) : base(lib::in_place, std::forward<Args>(args)...) {}

  template<
      typename U = T,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<T, std::initializer_list<U> &, Args...>())>
  constexpr explicit optional(lib::in_place_t, std::initializer_list<U> ilist, Args &&...args)
      : base(lib::in_place, ilist, std::forward<Args>(args)...) {}

  template<
      typename U = lib::remove_cvref_t<T>,
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<U>, lib::in_place_t>()),
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<U>, optional>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<T>, bool>() || !lib::is_specialization_of<U, lib::optional>()),
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>())>
  constexpr optional(U &&value) // NOLINT
      : optional(lib::in_place, std::forward<U>(value)) {}

  template<
      typename U = lib::remove_cvref_t<T>,
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<U>, lib::in_place_t>()),
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<U>, optional>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<T>, bool>() || !lib::is_specialization_of<U, lib::optional>()),
      HALCHECK_REQUIRE(!std::is_convertible<U &&, T>())>
  explicit constexpr optional(U &&value) : optional(lib::in_place, std::forward<U>(value)) {}

  optional &operator=(lib::nullopt_t) {
    reset();
    return *this;
  }

  template<
      typename U = lib::remove_cv_t<T>,
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<U>(), optional>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U>()),
      HALCHECK_REQUIRE(std::is_assignable<T &, U>()),
      HALCHECK_REQUIRE(!std::is_scalar<T>() || !std::is_same<lib::decay_t<U>, T>())>
  optional &operator=(U &&other) {
    if (*this)
      **this = std::forward<U>(other);
    else
      emplace(std::forward<U>(other));
    return *this;
  }

  template<
      typename U,
      HALCHECK_REQUIRE(!std::is_constructible<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(std::is_assignable<T, U &&>())>
  optional &operator=(optional<U> &&other) {
    if (!other)
      reset();
    else if (*this)
      **this = std::move(*other);
    else
      emplace(std::move(*other));
    return *this;
  }

  template<
      typename U,
      HALCHECK_REQUIRE(!std::is_constructible<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_constructible<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_convertible<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, const optional<U> &>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, optional<U> &&>()),
      HALCHECK_REQUIRE(!std::is_assignable<T, const optional<U> &&>()),
      HALCHECK_REQUIRE(std::is_constructible<T, const U &>()),
      HALCHECK_REQUIRE(std::is_assignable<T, const U &>())>
  optional &operator=(const optional<U> &other) {
    if (!other)
      reset();
    else if (*this)
      **this = *other;
    else
      emplace(*other);
    return *this;
  }

  using typename base::value_type;
  using base::operator bool;
  using base::operator*;
  using base::emplace;
  using base::reset;

  const value_type *operator->() const { return &**this; }
  value_type *operator->() { return &**this; }

  constexpr bool has_value() const noexcept { return bool(*this); }

  const T &value() const & {
    if (!has_value())
      throw lib::bad_optional_access();

    return **this;
  }

  T &value() & {
    if (!has_value())
      throw lib::bad_optional_access();

    return **this;
  }

  const T &&value() const && {
    if (!has_value())
      throw lib::bad_optional_access();

    return **this;
  }

  T &&value() && {
    if (!has_value())
      throw lib::bad_optional_access();

    return **this;
  }

  template<
      typename U = lib::remove_cv_t<T>,
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>() && std::is_copy_constructible<T>())>
  constexpr T value_or(U &&default_value) const & {
    return *this ? **this : static_cast<T>(std::forward<U>(default_value));
  }

  template<
      typename U = lib::remove_cv_t<T>,
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>() && std::is_move_constructible<T>())>
  T value_or(U &&default_value) && {
    return *this ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
  }

  template<bool _ = std::is_move_constructible<T>() && lib::is_swappable<T>(), HALCHECK_REQUIRE(_)>
  void swap(optional &other) noexcept(std::is_nothrow_move_constructible<T>() && lib::is_nothrow_swappable<T>()) {
    using std::swap;
    if (*this && other)
      swap(**this, *other);
    else if (*this) {
      other = std::move(*this);
      reset();
    } else {
      *this = std::move(other);
      other.reset();
    }
  }

#if __cplusplus >= 201606L
  constexpr operator std::optional<T>() const & { // NOLINT
    return *this ? std::optional<T>(std::in_place, **this) : std::optional<T>();
  }

  constexpr operator std::optional<T>() && { // NOLINT
    return *this ? std::optional<T>(std::in_place, std::move(**this)) : std::optional<T>();
  }
#endif

private:
  /// @brief An implementation of std::swap for @ref optional.
  /// @see std::swap
  template<bool _ = true, HALCHECK_REQUIRE(std::is_move_constructible<T>() && lib::is_swappable<T>() && _)>
  friend void swap(lib::optional<T> &lhs, lib::optional<T> &rhs) noexcept(
      std::is_nothrow_move_constructible<T>() && lib::is_nothrow_swappable<T>()) {
    lhs.swap(rhs);
  }
};

namespace detail {
template<typename T>
struct optional_void_base {
  using value_type = T;

  constexpr optional_void_base() noexcept : _has_value(false) {}
  explicit constexpr optional_void_base(lib::nullopt_t) noexcept : _has_value(false) {}
  explicit constexpr optional_void_base(lib::in_place_t) noexcept : _has_value(true) {}

  lib::optional<T> &operator=(lib::nullopt_t) noexcept { // NOLINT: returning reference to subclass
    _has_value = false;
    return *static_cast<lib::optional<T> *>(this);
  }

  explicit operator bool() const noexcept { return _has_value; }
  bool has_value() const noexcept { return _has_value; }
  void operator*() const noexcept {}
  void value() const {
    if (!_has_value)
      throw lib::bad_optional_access();
  }

  void swap(lib::optional<T> &other) noexcept { std::swap(_has_value, other._has_value); }
  void reset() noexcept { _has_value = false; }
  void emplace() noexcept { _has_value = true; }

private:
  /// @brief An implementation of std::swap for @ref optional.
  /// @see std::swap
  friend void swap(lib::optional<T> &lhs, lib::optional<T> &rhs) noexcept { lhs.swap(rhs); }

  bool _has_value;
};
} // namespace detail

/// @brief A specialization of @ref optional for @p "void".
/// @details Behaves like @p bool.
template<>
struct optional<void> : public detail::optional_void_base<void> {
  using detail::optional_void_base<void>::optional_void_base;
};

/// @brief A specialization of @ref optional for @p "const void".
/// @details Behaves like @p bool.
template<>
struct optional<const void> : public detail::optional_void_base<const void> {
  using detail::optional_void_base<const void>::optional_void_base;
};

/// @brief A specialization of @ref optional for @p "volatile void".
/// @details Behaves like @p bool.
template<>
struct optional<volatile void> : public detail::optional_void_base<volatile void> {
  using detail::optional_void_base<volatile void>::optional_void_base;
};

/// @brief A specialization of @ref optional for @p "const volatile void".
/// @details Behaves like @p bool.
template<>
struct optional<const volatile void> : public detail::optional_void_base<const volatile void> {
  using detail::optional_void_base<const volatile void>::optional_void_base;
};

/// @brief An implementation of std::make_optional.
/// @see std::make_optional
template<typename T>
constexpr lib::optional<lib::decay_t<T>> make_optional(T &&value) {
  return lib::optional<lib::decay_t<T>>(lib::in_place, std::forward<T>(value));
}

/// @brief An implementation of std::make_optional.
/// @see std::make_optional
template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(std::is_constructible<T, Args...>() || (std::is_void<T>() && sizeof...(Args) == 0))>
constexpr lib::optional<T> make_optional(Args &&...args) {
  return lib::optional<T>(lib::in_place, std::forward<T>(args)...);
}

/// @brief An implementation of std::make_optional.
/// @see std::make_optional
template<
    typename T,
    typename U,
    typename... Args,
    HALCHECK_REQUIRE(std::is_constructible<T, std::initializer_list<U>, Args...>())>
constexpr lib::optional<T> make_optional(const std::initializer_list<U> &ilist, Args &&...args) {
  return lib::optional<T>(lib::in_place, ilist, std::forward<T>(args)...);
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator==(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs == *rhs;
  else
    return !lhs && !rhs;
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator==(const lib::nullopt_t &, const lib::optional<T> &value) {
  return !value;
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator==(const lib::optional<T> &value, const lib::nullopt_t &) {
  return !value;
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator==(const U &lhs, const lib::optional<T> &rhs) {
  return rhs && lhs == *rhs;
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator==(const lib::optional<T> &lhs, const U &rhs) {
  return lhs && *lhs == rhs;
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator!=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs != *rhs;
  else
    return lhs || rhs;
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator!=(const lib::nullopt_t &, const lib::optional<T> &value) {
  return value;
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator!=(const lib::optional<T> &value, const lib::nullopt_t &) {
  return value;
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator!=(const U &lhs, const lib::optional<T> &rhs) {
  return !rhs || lhs != *rhs;
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator!=(const lib::optional<T> &lhs, const U &rhs) {
  return !lhs || *lhs != rhs;
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator<(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs < *rhs;
  else
    return !lhs && rhs;
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator<(const lib::nullopt_t &, const lib::optional<T> &value) {
  return value;
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator<(const lib::optional<T> &, const lib::nullopt_t &) {
  return false;
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator<(const U &lhs, const lib::optional<T> &rhs) {
  return rhs && lhs < *rhs;
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator<(const lib::optional<T> &lhs, const U &rhs) {
  return !lhs || *lhs < rhs;
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator>(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs > *rhs;
  else
    return lhs && !rhs;
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator>(const lib::nullopt_t &, const lib::optional<T> &) {
  return false;
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator>(const lib::optional<T> &value, const lib::nullopt_t &) {
  return value;
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator>(const U &lhs, const lib::optional<T> &rhs) {
  return !rhs || lhs > *rhs;
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator>(const lib::optional<T> &lhs, const U &rhs) {
  return lhs && *lhs < rhs;
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator<=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs <= *rhs;
  else
    return !lhs;
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator<=(const lib::nullopt_t &, const lib::optional<T> &) {
  return true;
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator<=(const lib::optional<T> &value, const lib::nullopt_t &) {
  return !value;
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator<=(const U &lhs, const lib::optional<T> &rhs) {
  return rhs && lhs <= *rhs;
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator<=(const lib::optional<T> &lhs, const U &rhs) {
  return !lhs || *lhs <= rhs;
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(!std::is_void<T>()), HALCHECK_REQUIRE(!std::is_void<U>())>
bool operator>=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  if (lhs && rhs)
    return *lhs >= *rhs;
  else
    return !rhs;
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator>=(const lib::nullopt_t &, const lib::optional<T> &value) {
  return !value;
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T>
bool operator>=(const lib::optional<T> &, const lib::nullopt_t &) {
  return true;
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator>=(const U &lhs, const lib::optional<T> &rhs) {
  return !rhs || lhs >= *rhs;
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U>
bool operator>=(const lib::optional<T> &lhs, const U &rhs) {
  return lhs && *lhs >= rhs;
}

/// @brief An implementation of std::optional::operator==.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator==(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) == bool(rhs);
}

/// @brief An implementation of std::optional::operator!=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator!=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) != bool(rhs);
}

/// @brief An implementation of std::optional::operator<.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator<(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) < bool(rhs);
}

/// @brief An implementation of std::optional::operator>.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator>(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) > bool(rhs);
}

/// @brief An implementation of std::optional::operator<=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator<=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) <= bool(rhs);
}

/// @brief An implementation of std::optional::operator>=.
/// @see https://en.cppreference.com/w/cpp/utility/optional/operator_cmp
template<typename T, typename U, HALCHECK_REQUIRE(std::is_void<T>()), HALCHECK_REQUIRE(std::is_void<U>())>
bool operator>=(const lib::optional<T> &lhs, const lib::optional<U> &rhs) {
  return bool(lhs) >= bool(rhs);
}

}} // namespace halcheck::lib

namespace std {

/// @brief A std::hash specialization for halcheck::lib::optional.
template<typename T>
struct hash<halcheck::lib::optional<T>> { // NOLINT
  template<typename U = T, HALCHECK_REQUIRE(halcheck::lib::is_hashable<U>())>
  std::size_t operator()(const halcheck::lib::optional<T> &value) const noexcept {
    return value ? std::hash<halcheck::lib::remove_const_t<T>>()(*value) : 0;
  }

  template<typename U = T, HALCHECK_REQUIRE(std::is_void<U>())>
  std::size_t operator()(const halcheck::lib::optional<T> &value) const noexcept {
    return std::hash<bool>()(bool(value));
  }
};
} // namespace std

#endif
