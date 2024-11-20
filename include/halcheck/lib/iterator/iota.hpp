#ifndef HALCHECK_LIB_ITERATOR_IOTA_HPP
#define HALCHECK_LIB_ITERATOR_IOTA_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/pp.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <iterator>

namespace halcheck { namespace lib {

/**
 * @brief Computes a type large enough to contain the difference between two instances of an iterator.
 * @tparam I The iterator type to compute a difference type for.
 * @ingroup lib-iterator
 */
template<typename I, typename = void>
struct iota_diff {};

template<typename I>
struct iota_diff<I, lib::enable_if_t<!std::is_integral<I>()>> {
  using type = lib::iter_difference_t<I>;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<(std::is_integral<I>() && sizeof(lib::iter_difference_t<I>) > sizeof(I))>> {
  using type = lib::iter_difference_t<I>;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<std::is_integral<I>() && (sizeof(std::int8_t) > sizeof(I))>> {
  using type = std::int8_t;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<std::is_integral<I>() && (sizeof(std::int16_t) > sizeof(I))>> {
  using type = std::int16_t;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<std::is_integral<I>() && (sizeof(std::int32_t) > sizeof(I))>> {
  using type = std::int32_t;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<std::is_integral<I>() && (sizeof(std::int64_t) > sizeof(I))>> {
  using type = std::int64_t;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<std::is_integral<I>() && sizeof(I) >= sizeof(std::int64_t)>> {
  using type = std::intmax_t;
};

/**
 * @brief Computes a type large enough to contain the difference between two instances of an iterator.
 * @tparam I The iterator type to compute a difference type for.
 * @ingroup lib-iterator
 */
template<typename I>
using iota_diff_t = typename iota_diff<I>::type;

/**
 * @brief An iterator pointing to an integral value.
 * @tparam T The type of value to point to.
 * @ingroup lib-iterator
 */
template<typename T>
class iota_iterator : private lib::iterator_interface<iota_iterator<T>> {
public:
  static_assert(std::is_integral<T>(), "T must be an integral type");

  using lib::iterator_interface<iota_iterator>::operator++;
  using lib::iterator_interface<iota_iterator>::operator--;
  using lib::iterator_interface<iota_iterator>::operator-=;
  using lib::iterator_interface<iota_iterator>::operator[];

  /**
   * @brief The type of value pointed to by this iterator.
   */
  using value_type = T;

  /**
   * @brief The return type of `operator*`.
   */
  using reference = T;

  /**
   * @brief This type does not support `operator->`.
   */
  using pointer = void;

  /**
   * @brief The return type of `operator-`.
   */
  using difference_type = lib::iota_diff_t<T>;

  /**
   * @brief This iterator is an @ref lib::is_input_iterator "input iterator".
   */
  using iterator_category = std::input_iterator_tag;

  /**
   * @brief Constructs a default @ref iota_iterator.
   */
  constexpr iota_iterator() = default;

  /**
   * @brief Constructs an @ref iota_iterator pointing to the specified value.
   * @param value The value to point to.
   */
  constexpr explicit iota_iterator(T value) : _value(std::move(value)) {}

  /**
   * @brief Obtains the value pointed to by this iterator.
   * @return The value pointed to by this iterator.
   */
  constexpr value_type operator*() const noexcept { return _value; }

  /**
   * @brief Advances this iterator.
   * @return A reference to `this`.
   */
  iota_iterator &operator++() {
    ++_value;
    return *this;
  }

  /**
   * @brief Advances this iterator backwards.
   * @return A reference to `this`.
   */
  iota_iterator &operator--() {
    --_value;
    return *this;
  }

  /**
   * @brief Advances this iterator by the specified amount.
   * @param n The amount to advance by.
   * @return A reference to `this`.
   */
  template<typename U = T, HALCHECK_REQUIRE(std::is_unsigned<U>())>
  iota_iterator &operator+=(difference_type n) {
    if (n >= 0)
      _value += static_cast<U>(n);
    else
      _value -= static_cast<U>(-n);
  }

  /**
   * @brief Advances this iterator by the specified amount.
   * @param n The amount to advance by.
   * @return A reference to `this`.
   */
  template<typename U = T, HALCHECK_REQUIRE(!std::is_unsigned<U>())>
  iota_iterator &operator+=(difference_type n) {
    _value += n;
  }

private:
  friend class lib::iterator_interface<iota_iterator>;

  /**
   * @brief Determines if two iterators are equal.
   * @param lhs, rhs The iterators to compare.
   * @return `*lhs == *rhs`
   */
  friend bool operator==(const iota_iterator &lhs, const iota_iterator &rhs) { return lhs._value == rhs._value; }

  /**
   * @brief Determines the distance between two iterators.
   * @param lhs, rhs The iterators to compare.
   * @return `*lhs - *rhs`
   */
  friend difference_type operator-(const iota_iterator &lhs, const iota_iterator &rhs) {
    using D = difference_type;

    if (std::is_signed<T>())
      return D(D(lhs._value) - D(rhs._value));
    else if (rhs._value > lhs._value)
      return D(-D(rhs._value) - D(lhs._value));
    else
      return D(lhs._value - rhs._value);
  }

  T _value;
};

/**
 * @brief Constructs an @ref iota_iterator.
 * @par Signature
 * @code
 *   template<typename T>
 *   lib::iota_iterator<T> make_iota_iterator(T value)
 * @endcode
 * @tparam T The type of value to point to.
 * @param value The value to point to.
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
  template<typename T>
  lib::iota_iterator<T> operator()(T value) const {
    return lib::iota_iterator<T>(std::move(value));
  }
} make_iota_iterator;

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
class iota_view : public lib::view_interface<iota_view<T>> {
public:
  iota_view() = default;
  iota_view(T begin, T end) : _begin(std::move(begin)), _end(std::move(end)) {}

  iota_iterator<T> begin() const { return iota_iterator<T>(_begin); }
  iota_iterator<T> end() const { return iota_iterator<T>(_end); }

  lib::make_unsigned_t<T> size() const {
    return _begin < 0 ? (_end < 0 ? lib::to_unsigned(-_begin) - lib::to_unsigned(-_end)
                                  : lib::to_unsigned(_end) + lib::to_unsigned(-_begin))
                      : lib::to_unsigned(_end) - lib::to_unsigned(_begin);
  }

private:
  T _begin{};
  T _end{};
};

HALCHECK_INLINE_CONSTEXPR struct {
  template<typename T>
  lib::iota_view<T> operator()(T begin, T end) const {
    return lib::iota_view<T>(std::move(begin), std::move(end));
  }

  template<typename T>
  lib::iota_view<T> operator()(T end) const {
    return lib::iota_view<T>(0, std::move(end));
  }
} iota;

}} // namespace halcheck::lib

#endif
