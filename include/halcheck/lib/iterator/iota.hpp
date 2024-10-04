#ifndef HALCHECK_LIB_ITERATOR_IOTA_HPP
#define HALCHECK_LIB_ITERATOR_IOTA_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/incrementable.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <iterator>

namespace halcheck { namespace lib {

template<typename I, typename = void>
struct iota_diff {};

template<typename I>
struct iota_diff<I, lib::enable_if_t<!std::is_integral<I>()>> {
  using type = lib::iter_difference_t<I>;
};

template<typename I>
struct iota_diff<I, lib::enable_if_t<(std::is_integral<I>() && sizeof(lib::iter_difference_t<I>()) > sizeof(I))>> {
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

template<typename I>
using iota_diff_t = typename iota_diff<I>::type;

namespace detail {
template<typename T, typename = void>
struct iota_iterator_base {};

template<typename T>
struct iota_iterator_base<T, lib::to_void<lib::incrementable<T>, lib::integral<lib::iota_diff_t<T>>>> {
  using iterator_category = std::input_iterator_tag;
};
} // namespace detail

template<typename T, typename = lib::to_void<lib::weakly_incrementable<T>, lib::copyable<T>>>
class iota_iterator : private lib::iterator_interface<iota_iterator<T>>, public detail::iota_iterator_base<T> {
public:
  using lib::iterator_interface<iota_iterator>::operator++;
  using lib::iterator_interface<iota_iterator>::operator--;
  using lib::iterator_interface<iota_iterator>::operator[];

  using value_type = T;
  using difference_type = lib::iota_diff_t<T>;
  using iterator_concept = lib::conditional_t<
      lib::is_advanceable<T, difference_type>{},
      std::random_access_iterator_tag,
      lib::conditional_t<
          lib::is_decrementable<T>{},
          std::bidirectional_iterator_tag,
          lib::conditional_t<lib::is_incrementable<T>{}, std::forward_iterator_tag, std::input_iterator_tag>>>;

  constexpr iota_iterator() = default;

  constexpr explicit iota_iterator(T value) : _value(std::move(value)) {}

  constexpr T operator*() const noexcept(std::is_nothrow_copy_constructible<T>()) { return _value; }

  iota_iterator &operator++() {
    ++_value;
    return *this;
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_decrementable<U>())>
  iota_iterator &operator--() {
    --_value;
    return *this;
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(lib::is_unsigned_integral<U>())>
  iota_iterator &operator+=(difference_type n) {
    if (n >= 0)
      _value += static_cast<U>(n);
    else
      _value -= static_cast<U>(-n);
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(!lib::is_unsigned_integral<U>())>
  iota_iterator &operator+=(difference_type n) {
    _value += n;
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(lib::is_unsigned_integral<U>())>
  iota_iterator &operator-=(difference_type n) {
    if (n >= 0)
      _value -= static_cast<U>(n);
    else
      _value += static_cast<U>(-n);
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(!lib::is_unsigned_integral<U>())>
  iota_iterator &operator-=(difference_type n) {
    _value -= n;
  }

private:
  template<typename, typename, typename>
  friend class iota_sentinel;
  friend class lib::iterator_interface<iota_iterator>;

  template<typename U = T, HALCHECK_REQUIRE(lib::is_equality_comparable<U>())>
  friend bool operator==(const iota_iterator &lhs, const iota_iterator &rhs) {
    return lhs._value == rhs._value;
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_totally_ordered<U>())>
  friend bool operator<(const iota_iterator &lhs, const iota_iterator &rhs) {
    return lhs._value < rhs._value;
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(std::is_integral<U>())>
  friend difference_type operator-(const iota_iterator &lhs, const iota_iterator &rhs) {
    using D = difference_type;

    if (std::is_signed<T>())
      return D(D(lhs._value) - D(rhs._value));
    else if (rhs > lhs)
      return D(-D(rhs._value) - D(lhs._value));
    else
      return D(lhs._value - rhs._value);
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(lib::is_advanceable<U, difference_type>()),
      HALCHECK_REQUIRE(!std::is_integral<U>())>
  friend difference_type operator-(const iota_iterator &lhs, const iota_iterator &rhs) {
    return lhs._value - rhs._value;
  }

  T _value;
};

template<typename T>
lib::iota_iterator<T> make_iota_iterator(T value) {
  return lib::iota_iterator<T>(std::move(value));
}

}} // namespace halcheck::lib

#endif
