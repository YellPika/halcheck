#ifndef HALCHECK_LIB_ITERATOR_INTERFACE_HPP
#define HALCHECK_LIB_ITERATOR_INTERFACE_HPP

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

namespace detail {
template<typename I>
using dereference = decltype(*std::declval<I &>());

template<typename I>
using equality = lib::boolean_testable<decltype(std::declval<const I &>() == std::declval<const I &>())>;

template<typename I, typename D = decltype(std::declval<const I &>() - std::declval<const I &>())>
using difference = lib::enable_if_t<std::is_integral<D>() && std::is_signed<D>()>;

template<typename I>
using increment = decltype(++std::declval<I &>());

template<typename I>
using decrement = decltype(--std::declval<I &>());

template<typename I, typename N>
using addition = lib::same<decltype(std::declval<I &>() += std::declval<const N &>()), I &>;
} // namespace detail

template<typename I>
class iterator_interface {
protected:
  iterator_interface() = default;

public:
  template<
      typename J = I,
      HALCHECK_REQUIRE(lib::is_detected<detail::dereference, J>()),
      HALCHECK_REQUIRE(lib::is_detected<detail::addition, J, lib::iter_difference_t<J>>())>
  constexpr lib::iter_reference_t<J> operator[](lib::iter_difference_t<J> n) const {
    return *(static_cast<const J &>(*this) + n);
  }

  template<
      typename J = I,
      HALCHECK_REQUIRE(lib::is_detected<detail::increment, J>()),
      HALCHECK_REQUIRE(lib::is_copyable<J>())>
  J operator++(int) {
    J &self = *static_cast<J *>(this);
    auto output = self;
    ++self;
    return output;
  }

  template<
      typename J = I,
      HALCHECK_REQUIRE(lib::is_detected<detail::increment, J>()),
      HALCHECK_REQUIRE(!lib::is_copyable<J>())>
  void operator++(int) {
    ++*static_cast<J *>(this);
  }

  template<
      typename J = I,
      HALCHECK_REQUIRE(lib::is_detected<detail::decrement, J>()),
      HALCHECK_REQUIRE(lib::is_copyable<J>())>
  J operator--(int) {
    J &self = *static_cast<J *>(this);
    auto output = self;
    --self;
    return output;
  }

  template<
      typename J = I,
      HALCHECK_REQUIRE(lib::is_detected<detail::decrement, J>()),
      HALCHECK_REQUIRE(!lib::is_copyable<J>())>
  void operator--(int) {
    --*static_cast<J *>(this);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::addition, J, lib::iter_difference_t<J>>())>
  J &operator-=(lib::iter_difference_t<J> n) {
    J &self = *static_cast<J *>(this);
    self += -n;
    return self;
  }

private:
  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::equality, J>())>
  friend bool operator!=(const I &lhs, const I &rhs) {
    return !bool(lhs == rhs);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::difference, J>())>
  friend bool operator<(const I &lhs, const I &rhs) {
    return (lhs - rhs) < 0;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::difference, J>())>
  friend bool operator>(const I &lhs, const I &rhs) {
    return rhs < lhs;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::difference, J>())>
  friend bool operator<=(const I &lhs, const I &rhs) {
    return !(rhs < lhs);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::difference, J>())>
  friend bool operator>=(const I &lhs, const I &rhs) {
    return !(lhs < rhs);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::addition, J, lib::iter_difference_t<J>>())>
  friend I operator+(I i, lib::iter_difference_t<J> n) {
    i += n;
    return i;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::addition, J, lib::iter_difference_t<J>>())>
  friend I operator+(lib::iter_difference_t<J> n, I i) {
    i += n;
    return i;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::addition, J, lib::iter_difference_t<J>>())>
  friend I operator-(I i, lib::iter_difference_t<J> n) {
    i -= n;
    return i;
  }
};

// See https://en.cppreference.com/w/cpp/ranges/view_interface

template<typename T, HALCHECK_REQUIRE(std::is_class<T>()), HALCHECK_REQUIRE(std::is_same<T, lib::remove_cv_t<T>>())>
class view_interface {
private:
  template<typename U = T>
  U &self() {
    return static_cast<U &>(*this);
  }

  template<typename U = T>
  const U &self() const {
    return static_cast<U &>(*this);
  }

public:
  template<typename U = T, HALCHECK_REQUIRE(lib::is_sized_range<const U>())>
  constexpr bool empty() const {
    return lib::size(self()) == 0;
  }

  template<typename U = T, HALCHECK_REQUIRE(!lib::is_sized_range<const U>() && lib::is_forward_range<const U>())>
  constexpr bool empty() const {
    return lib::begin(self()) == lib::end(self());
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_sized_range<U>())>
  bool empty() {
    return lib::size(self()) == 0;
  }

  template<typename U = T, HALCHECK_REQUIRE(!lib::is_sized_range<U>() && lib::is_forward_range<U>())>
  bool empty() {
    return lib::begin(self()) == lib::end(self());
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_sized_range<const U>() || lib::is_forward_range<const U>())>
  constexpr explicit operator bool() const {
    return !empty();
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_sized_range<U>() || lib::is_forward_range<U>())>
  explicit operator bool() {
    return !empty();
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_random_access_range<const U>())>
  constexpr auto size() const -> decltype(lib::to_unsigned(lib::end(self<U>()) - lib::begin(self<U>()))) {
    return lib::to_unsigned(lib::end(self()) - lib::begin(self()));
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_random_access_range<U>())>
  auto size() -> decltype(lib::to_unsigned(lib::end(self<U>()) - lib::begin(self<U>()))) {
    return lib::to_unsigned(lib::end(self()) - lib::begin(self()));
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_forward_range<const U>())>
  constexpr auto front() const -> decltype(*lib::begin(self<U>())) {
    return *lib::begin(self());
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_forward_range<U>())>
  auto front() -> decltype(*lib::begin(self<U>())) {
    return *lib::begin(self());
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_bidirectional_range<const U>())>
  constexpr auto back() const -> decltype(*std::prev(lib::end(self<U>()))) {
    return *std::prev(lib::end(self()));
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_bidirectional_range<U>())>
  auto back() -> decltype(*std::prev(lib::end(self<U>()))) {
    return *std::prev(lib::end(self()));
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_random_access_range<const U>())>
  constexpr auto operator[](lib::range_difference_t<U> n) const -> decltype(lib::begin(self<U>())[n]) {
    return lib::begin(self<U>())[n];
  }

  template<typename U = T, HALCHECK_REQUIRE(lib::is_random_access_range<U>())>
  auto operator[](lib::range_difference_t<U> n) -> decltype(lib::begin(self<U>())[n]) {
    return lib::begin(self<U>())[n];
  }
};

}} // namespace halcheck::lib

#endif
