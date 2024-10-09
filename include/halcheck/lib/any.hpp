#ifndef HALCHECK_LIB_ANY_HPP
#define HALCHECK_LIB_ANY_HPP

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

/// @brief See https://en.cppreference.com/w/cpp/utility/any.
/// @ingroup util
class any {
public:
  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  constexpr any() noexcept = default;

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  any(const any &other) : _type(other._type), _impl(other.has_value() ? other._impl->clone() : nullptr) {}

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  any(any &&other) noexcept(false) : _type(other._type), _impl(other.has_value() ? other._impl->move() : nullptr) {}

  /// @fn template<typename T> any(T &&value)
  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_specialization_of<lib::decay_t<T>, lib::in_place_type_t>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  any(T &&value) // NOLINT: implicit conversion
      : any(lib::in_place_type_t<lib::decay_t<T>>(), std::forward<T>(value)) {}

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  explicit any(lib::in_place_type_t<T>, Args &&...args)
      : _type(lib::type_id::make<T>()), _impl(new derived<T>(std::forward<Args>(args)...)) {}

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/any.
  template<
      typename T,
      typename U,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, std::initializer_list<U> &, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  explicit any(lib::in_place_type_t<T>, std::initializer_list<U> il, Args &&...args)
      : _type(lib::type_id::make<T>()), _impl(new derived<T>(il, std::forward<Args>(args)...)) {}

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/%7Eany.
  ~any() = default;

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/operator%3D.
  any &operator=(const any &rhs) {
    any(rhs).swap(*this);
    return *this;
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/operator%3D.
  any &operator=(any &&rhs) noexcept {
    swap(rhs);
    return *this;
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/operator%3D.
  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_specialization_of<lib::decay_t<T>, lib::in_place_type_t>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  any &operator=(T &&rhs) noexcept {
    any(std::forward<T>(rhs)).swap(*this);
    return *this;
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/emplace.
  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  T &emplace(Args &&...args) {
    _impl.reset(new derived<T>(std::forward<Args>(args)...));
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/emplace.
  template<
      typename T,
      typename U,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<lib::decay_t<T>, std::initializer_list<U> &, Args...>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<lib::decay_t<T>>())>
  T &emplace(std::initializer_list<U> il, Args &&...args) {
    _impl.reset(new derived<T>(il, std::forward<Args>(args)...));
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/reset.
  void reset() noexcept {
    _type = lib::type_id::make<void>();
    _impl.reset();
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/swap.
  void swap(any &other) noexcept {
    using std::swap;
    swap(_type, other._type);
    swap(_impl, other._impl);
  }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/has_value.
  constexpr bool has_value() const noexcept { return bool(_impl); }

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/type.
  constexpr const lib::type_id &type() const { return _type; }

private:
  template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
  friend const T *any_cast(const any *operand) noexcept;

  template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
  friend T *any_cast(any *operand) noexcept;

  /// @brief See https://en.cppreference.com/w/cpp/utility/any/swap2.
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

/// @brief See https://en.cppreference.com/w/cpp/utility/any/bad_any_cast.
/// @relates any
struct bad_any_cast : std::exception {
  const char *what() const noexcept override { return "halcheck::lib::bad_any_cast"; }
};

/// @brief See https://en.cppreference.com/w/cpp/utility/any/any_cast.
/// @relates any
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, const lib::remove_cv_t<lib::remove_reference_t<T>> &>())>
T any_cast(const any &operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(*output);
  else
    throw lib::bad_any_cast();
}

/// @brief See https://en.cppreference.com/w/cpp/utility/any/any_cast.
/// @relates any
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cv_t<lib::remove_reference_t<T>> &>())>
T any_cast(any &operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(*output);
  else
    throw lib::bad_any_cast();
}

/// @brief See https://en.cppreference.com/w/cpp/utility/any/any_cast.
/// @relates any
template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cv_t<lib::remove_reference_t<T>>>())>
T any_cast(any &&operand) {
  if (auto output = lib::any_cast<lib::remove_cv_t<lib::remove_reference_t<T>>>(&operand))
    return static_cast<T>(std::move(*output));
  else
    throw lib::bad_any_cast();
}

/// @brief See https://en.cppreference.com/w/cpp/utility/any/any_cast.
template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
const T *any_cast(const any *operand) noexcept {
  if (operand && operand->type() == lib::type_id::make<T>())
    return reinterpret_cast<const T *>(operand->_impl->data());
  else
    return nullptr;
}

/// @brief See https://en.cppreference.com/w/cpp/utility/any/any_cast.
template<typename T, HALCHECK_REQUIRE_(!std::is_void<T>())>
T *any_cast(any *operand) noexcept {
  if (operand && operand->type() == lib::type_id::make<T>())
    return reinterpret_cast<T *>(operand->_impl->data());
  else
    return nullptr;
}

/// @brief See https://en.cppreference.com/w/cpp/utility/any/make_any.
/// @relates any
template<typename T, typename... Args>
any make_any(Args &&...args) {
  return any(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

}} // namespace halcheck::lib

#endif
