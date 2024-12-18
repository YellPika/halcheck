#ifndef HALCHECK_LIB_ANY_HPP
#define HALCHECK_LIB_ANY_HPP

/**
 * @defgroup lib-any lib/any
 * @brief An implementation of std::any.
 * @see https://en.cppreference.com/w/cpp/header/any
 * @ingroup lib
 */

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/typeinfo.hpp>
#include <halcheck/lib/variant.hpp>

#include <exception>
#include <initializer_list>
#include <memory>
#include <utility>

namespace halcheck { namespace lib {

class any;

template<typename T, HALCHECK_REQUIRE(!std::is_void<T>())>
T *any_cast(any *operand) noexcept;

template<typename T, HALCHECK_REQUIRE(!std::is_void<T>())>
const T *any_cast(const any *operand) noexcept;

/**
 * @brief An implemenetation of std::any.
 * @see std::any
 * @ingroup lib-any
 */
class any {
public:
  constexpr any() noexcept = default;

  any(const any &other) : _type(other._type), _impl(other.has_value() ? other._impl->clone() : nullptr) {}

  any(any &&other) noexcept(false) : _type(other._type), _impl(other.has_value() ? other._impl->move() : nullptr) {}

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_specialization_of<lib::decay_t<T>, lib::in_place_type_t>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  any(T &&value) // NOLINT: implicit conversion
      : any(lib::in_place_type_t<lib::decay_t<T>>(), std::forward<T>(value)) {}

  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  explicit any(lib::in_place_type_t<T>, Args &&...args)
      : _type(lib::type_id::of<T>()), _impl(new derived<T>(std::forward<Args>(args)...)) {}

  template<
      typename T,
      typename U,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, std::initializer_list<U> &, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  explicit any(lib::in_place_type_t<T>, std::initializer_list<U> il, Args &&...args)
      : _type(lib::type_id::of<T>()), _impl(new derived<T>(il, std::forward<Args>(args)...)) {}

  ~any() = default;

  any &operator=(const any &rhs) {
    any(rhs).swap(*this);
    return *this;
  }

  any &operator=(any &&rhs) noexcept {
    swap(rhs);
    return *this;
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_specialization_of<lib::decay_t<T>, lib::in_place_type_t>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  any &operator=(T &&rhs) noexcept {
    any(std::forward<T>(rhs)).swap(*this);
    return *this;
  }

  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  T &emplace(Args &&...args) {
    _impl.reset(new derived<T>(std::forward<Args>(args)...));
  }

  template<
      typename T,
      typename U,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, std::initializer_list<U> &, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  T &emplace(std::initializer_list<U> il, Args &&...args) {
    _impl.reset(new derived<T>(il, std::forward<Args>(args)...));
  }

  void reset() noexcept {
    _type = lib::type_id::of<void>();
    _impl.reset();
  }

  void swap(any &other) noexcept {
    using std::swap;
    swap(_type, other._type);
    swap(_impl, other._impl);
  }

  bool has_value() const noexcept { return bool(_impl); }

  const lib::type_id &type() const { return _type; }

private:
  template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
  friend const T *any_cast(const any *operand) noexcept;

  template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
  friend T *any_cast(any *operand) noexcept;

  friend void swap(any &lhs, any &rhs) noexcept { lhs.swap(rhs); }

  struct base {
    virtual ~base() = 0;
    virtual const void *data() const = 0;
    virtual void *data() = 0;
    virtual std::unique_ptr<base> move() = 0;
    virtual std::unique_ptr<base> clone() const = 0;
  };

  template<typename T>
  struct derived : base {
    template<typename... Args>
    explicit derived(Args &&...args) : value(std::forward<Args>(args)...) {}

    const void *data() const override { return std::addressof(value); }

    void *data() override { return std::addressof(value); }

    std::unique_ptr<base> move() override { return new derived<T>(std::move(value)); }

    std::unique_ptr<base> clone() const override { return new derived<T>(value); }

    T value;
  };

  lib::type_id _type;
  std::unique_ptr<base> _impl;
};

/**
 * @brief An implementation of std::bad_any_cast.
 * @see std::bad_any_cast
 * @relates any
 * @ingroup lib-any
 */
struct bad_any_cast : std::exception {
  const char *what() const noexcept override { return "halcheck::lib::bad_any_cast"; }
};

/**
 * @brief An implementation of std::any_cast.
 * @see std::any_cast
 * @relates any
 * @ingroup lib-any
 */
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, const lib::remove_cv_t<lib::remove_reference_t<T>> &>())>
T any_cast(const any &operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(*output);
  else
    throw lib::bad_any_cast();
}

/**
 * @brief An implementation of std::any_cast.
 * @see std::any_cast
 * @relates any
 * @ingroup lib-any
 */
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cv_t<lib::remove_reference_t<T>> &>())>
T any_cast(any &operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(*output);
  else
    throw lib::bad_any_cast();
}

/**
 * @brief An implementation of std::any_cast.
 * @see std::any_cast
 * @relates any
 * @ingroup lib-any
 */
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cv_t<lib::remove_reference_t<T>>>())>
T any_cast(any &&operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(std::move(*output));
  else
    throw lib::bad_any_cast();
}

/**
 * @brief An implementation of std::any_cast.
 * @see std::any_cast
 * @relates any
 * @ingroup lib-any
 */
template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
const T *any_cast(const any *operand) noexcept {
  if (operand && operand->type() == lib::type_id::of<T>())
    return reinterpret_cast<const T *>(operand->_impl->data());
  else
    return nullptr;
}

/**
 * @brief An implementation of std::any_cast.
 * @see std::any_cast
 * @relates any
 * @ingroup lib-any
 */
template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
T *any_cast(any *operand) noexcept {
  if (operand && operand->type() == lib::type_id::of<T>())
    return reinterpret_cast<T *>(operand->_impl->data());
  else
    return nullptr;
}

/**
 * @brief An implementation of std::make_any.
 * @see std::make_any
 * @relates any
 * @ingroup lib-any
 */
template<typename T, typename... Args>
any make_any(Args &&...args) {
  return any(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

}} // namespace halcheck::lib

#endif
