#ifndef HALCHECK_LIB_ITERATOR_INTEGRAL_HPP
#define HALCHECK_LIB_ITERATOR_INTEGRAL_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/ranges.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
struct integral_iterator {
  template<typename U, HALCHECK_REQUIRE_(std::is_integral<U>())>
  friend struct integral_iterator;

  using difference_type = lib::make_signed_t<T>;
  using reference = const T &;
  using value_type = T;
  using pointer = const T *;
  using iterator_category = std::random_access_iterator_tag;

  template<typename U = T, HALCHECK_REQUIRE(std::is_convertible<U, T>())>
  integral_iterator(U value = T(0)) // NOLINT: implicit conversion
      : _value(value) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(!std::is_convertible<U, T>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U>())>
  explicit integral_iterator(U value) // NOLINT: implicit conversion
      : _value(value) {}

  template<typename U, HALCHECK_REQUIRE(std::is_convertible<U, T>())>
  integral_iterator(lib::integral_iterator<U> other) // NOLINT: implicit conversion
      : _value(other._value) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(!std::is_convertible<U, T>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U>())>
  explicit integral_iterator(lib::integral_iterator<U> other) // NOLINT: implicit conversion
      : _value(other._value) {}

  reference operator*() const { return _value; }

  pointer operator->() const { return &_value; }

  integral_iterator &operator++() {
    ++_value;
    return *this;
  }

  integral_iterator operator++(int) {
    auto output = *this;
    ++_value;
    return output;
  }

  integral_iterator &operator+=(difference_type n) {
    _value += n;
    return *this;
  }

  integral_iterator &operator-=(difference_type n) {
    _value -= n;
    return *this;
  }

  integral_iterator &operator--() {
    --_value;
    return *this;
  }

  integral_iterator operator--(int) {
    auto output = *this;
    --_value;
    return output;
  }

  reference operator[](difference_type n) const { return *(*this + n); }

  friend bool operator==(const integral_iterator &lhs, const integral_iterator &rhs) {
    return lhs._value == rhs._value;
  }

  friend bool operator!=(const integral_iterator &lhs, const integral_iterator &rhs) { return !(lhs == rhs); }

  friend bool operator<(const integral_iterator &lhs, const integral_iterator &rhs) { return lhs._value < rhs._value; }

  friend bool operator>(const integral_iterator &lhs, const integral_iterator &rhs) { return lhs._value > rhs._value; }

  friend bool operator<=(const integral_iterator &lhs, const integral_iterator &rhs) { return !(lhs > rhs); }

  friend bool operator>=(const integral_iterator &lhs, const integral_iterator &rhs) { return !(lhs < rhs); }

  friend integral_iterator operator+(integral_iterator lhs, difference_type rhs) { return lhs += rhs; }

  friend integral_iterator operator+(difference_type lhs, integral_iterator rhs) { return rhs += lhs; }

  friend integral_iterator operator-(integral_iterator lhs, difference_type rhs) { return lhs -= rhs; }

  friend difference_type operator-(integral_iterator lhs, integral_iterator rhs) { return lhs._value - rhs._value; }

private:
  T _value;
};

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
lib::integral_iterator<T> make_integral_iterator(T value) {
  return lib::integral_iterator<T>(value);
}

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
using integral_view = lib::view<lib::integral_iterator<T>>;

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
lib::integral_view<T> make_integral_view(T begin, T end) {
  return lib::integral_view<T>(begin, end);
}

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
lib::integral_view<T> make_integral_view(T end) {
  return lib::integral_view<T>(T(0), end);
}

}} // namespace halcheck::lib

#endif
