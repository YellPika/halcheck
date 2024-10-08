#ifndef HALCHECK_LIB_ITERATOR_IOTA_HPP
#define HALCHECK_LIB_ITERATOR_IOTA_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
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

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
class iota_iterator : private lib::iterator_interface<iota_iterator<T>> {
public:
  using lib::iterator_interface<iota_iterator>::operator++;
  using lib::iterator_interface<iota_iterator>::operator--;
  using lib::iterator_interface<iota_iterator>::operator-=;

  using value_type = T;
  using reference = const T &;
  using pointer = void;
  using difference_type = lib::iota_diff_t<T>;
  using iterator_category = std::random_access_iterator_tag;

  constexpr iota_iterator() = default;

  constexpr explicit iota_iterator(T value) : _value(std::move(value)) {}

  constexpr reference operator*() const noexcept { return _value; }

  constexpr value_type operator[](difference_type n) const noexcept { return _value + n; }

  iota_iterator &operator++() {
    ++_value;
    return *this;
  }

  iota_iterator &operator--() {
    --_value;
    return *this;
  }

  template<typename U = T, HALCHECK_REQUIRE(std::is_unsigned<U>())>
  iota_iterator &operator+=(difference_type n) {
    if (n >= 0)
      _value += static_cast<U>(n);
    else
      _value -= static_cast<U>(-n);
  }

  template<typename U = T, HALCHECK_REQUIRE(!std::is_unsigned<U>())>
  iota_iterator &operator+=(difference_type n) {
    _value += n;
  }

private:
  friend class lib::iterator_interface<iota_iterator>;

  friend bool operator==(const iota_iterator &lhs, const iota_iterator &rhs) { return lhs._value == rhs._value; }

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

template<typename T>
lib::iota_iterator<T> make_iota_iterator(T value) {
  return lib::iota_iterator<T>(std::move(value));
}

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

static const struct {
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
