#ifndef HALCHECK_LIB_ITERATOR_INTERFACE_HPP
#define HALCHECK_LIB_ITERATOR_INTERFACE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/range.hpp>
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

/**
 * @brief A utility class for easily defining new iterators.
 * @tparam Self The derived type (as used in
 * [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)).
 * @ingroup lib-iterator
 */
template<typename Self>
class iterator_interface {
protected:
  iterator_interface() = default;

public:
  /**
   * @brief Access the element at the specified index.
   * @param n The index of the element to access.
   * @return A reference to the specified element.
   * @details This overload participates in overload resolution only if `*(*this + n)` is a valid expression.
   */
  template<
      typename I = Self,
      HALCHECK_REQUIRE(lib::is_detected<detail::dereference, I>()),
      HALCHECK_REQUIRE(lib::is_detected<detail::addition, I, lib::iter_difference_t<I>>())>
  constexpr lib::iter_reference_t<I> operator[](lib::iter_difference_t<I> n) const {
    return *(static_cast<const I &>(*this) + n);
  }

  /**
   * @brief Advances this iterator.
   * @return A copy of this iterator prior to advancement.
   * @details This overload participates in overload resolution only if `Self` is copyable and `++*this` is a valid
   * expression.
   */
  template<
      typename I = Self,
      HALCHECK_REQUIRE(lib::is_detected<detail::increment, I>()),
      HALCHECK_REQUIRE(lib::is_copyable<I>())>
  I operator++(int) {
    I &self = *static_cast<I *>(this);
    auto output = self;
    ++self;
    return output;
  }

  /**
   * @brief Advances this iterator.
   * @details This overload participates in overload resolution only if `Self` is not copyable and `++*this` is a valid
   * expression.
   */
  template<
      typename I = Self,
      HALCHECK_REQUIRE(lib::is_detected<detail::increment, I>()),
      HALCHECK_REQUIRE(!lib::is_copyable<I>())>
  void operator++(int) {
    ++*static_cast<I *>(this);
  }

  /**
   * @brief Advances this iterator a step backwards.
   * @return A copy of this iterator prior to advancement.
   * @details This overload participates in overload resolution only if `Self` is copyable and `--*this` is a valid
   * expression.
   */
  template<
      typename I = Self,
      HALCHECK_REQUIRE(lib::is_detected<detail::decrement, I>()),
      HALCHECK_REQUIRE(lib::is_copyable<I>())>
  I operator--(int) {
    I &self = *static_cast<I *>(this);
    auto output = self;
    --self;
    return output;
  }

  /**
   * @brief Advances this iterator a step backwards.
   * @details This overload participates in overload resolution only if `Self` is not copyable and `--*this` is a valid
   * expression.
   */
  template<
      typename I = Self,
      HALCHECK_REQUIRE(lib::is_detected<detail::decrement, I>()),
      HALCHECK_REQUIRE(!lib::is_copyable<I>())>
  void operator--(int) {
    --*static_cast<I *>(this);
  }

  /**
   * @brief Advances this iterator the specified number of steps backwards.
   * @param n The number of steps to advance.
   * @return A reference to `this`.
   * @details This overload participates in overload resolution only if `Self` is not copyable and `*this += -n` is a
   * valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::addition, I, lib::iter_difference_t<I>>())>
  I &operator-=(lib::iter_difference_t<I> n) {
    I &self = *static_cast<I *>(this);
    self += -n;
    return self;
  }

private:
  /**
   * @brief Determines if two iterators are not equal.
   * @param lhs,rhs The iterators to compare.
   * @return `!bool(lhs == rhs)`
   * @details This overload participates in overload resolution only if `lhs == rhs` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::equality, I>())>
  friend bool operator!=(const Self &lhs, const Self &rhs) {
    return !bool(lhs == rhs);
  }

  /**
   * @brief Determines if one iterator preceeds another.
   * @param lhs,rhs The iterators to compare.
   * @return `(lhs - rhs) < 0`
   * @details This overload participates in overload resolution only if `lhs - rhs` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::difference, I>())>
  friend bool operator<(const Self &lhs, const Self &rhs) {
    return (lhs - rhs) < 0;
  }

  /**
   * @brief Determines if one iterator succeeds another.
   * @param lhs,rhs The iterators to compare.
   * @return `rhs < lhs`
   * @details This overload participates in overload resolution only if `lhs - rhs` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::difference, I>())>
  friend bool operator>(const Self &lhs, const Self &rhs) {
    return rhs < lhs;
  }

  /**
   * @brief Determines if one iterator either preceeds or is equal to another.
   * @param lhs,rhs The iterators to compare.
   * @return `!(rhs < lhs)`
   * @details This overload participates in overload resolution only if `lhs - rhs` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::difference, I>())>
  friend bool operator<=(const Self &lhs, const Self &rhs) {
    return !(rhs < lhs);
  }

  /**
   * @brief Determines if one iterator either succeeds or is equal to another.
   * @param lhs,rhs The iterators to compare.
   * @return `!(rhs < lhs)`
   * @details This overload participates in overload resolution only if `lhs - rhs` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::difference, I>())>
  friend bool operator>=(const Self &lhs, const Self &rhs) {
    return !(lhs < rhs);
  }

  /**
   * @brief Advances an iterator by a specified amount.
   * @param i The iterator to advance.
   * @param n The amount to advance by.
   * @return An advanced copy of @p i.
   * @details This overload participates in overload resolution only if `i += n` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::addition, I, lib::iter_difference_t<I>>())>
  friend Self operator+(Self i, lib::iter_difference_t<I> n) {
    i += n;
    return i;
  }

  /**
   * @brief Advances an iterator by a specified amount.
   * @param n The amount to advance by.
   * @param i The iterator to advance.
   * @return An advanced copy of @p i.
   * @details This overload participates in overload resolution only if `i += n` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::addition, I, lib::iter_difference_t<I>>())>
  friend Self operator+(lib::iter_difference_t<I> n, Self i) {
    i += n;
    return i;
  }

  /**
   * @brief Advances an iterator backwards by a specified amount.
   * @param n The amount to advance by.
   * @param i The iterator to advance.
   * @return An advanced copy of @p i.
   * @details This overload participates in overload resolution only if `i += n` is a valid expression.
   */
  template<typename I = Self, HALCHECK_REQUIRE(lib::is_detected<detail::addition, I, lib::iter_difference_t<I>>())>
  friend Self operator-(Self i, lib::iter_difference_t<I> n) {
    i -= n;
    return i;
  }
};

/**
 * @brief An implementation of std::ranges::view_interface.
 * @see std::ranges::view_interface
 * @ingroup lib-iterator
 */
template<
    typename Self,
    HALCHECK_REQUIRE(std::is_class<Self>()),
    HALCHECK_REQUIRE(std::is_same<Self, lib::remove_cv_t<Self>>())>
class view_interface : public lib::view_base {
private:
  template<typename T = Self>
  T &self() {
    return static_cast<T &>(*this);
  }

  template<typename T = Self>
  const T &self() const {
    return static_cast<const T &>(*this);
  }

public:
  template<typename T = Self, HALCHECK_REQUIRE(lib::is_sized_range<const T>())>
  constexpr bool empty() const {
    return lib::size(self()) == 0;
  }

  template<typename T = Self, HALCHECK_REQUIRE(!lib::is_sized_range<const T>() && lib::is_forward_range<const T>())>
  constexpr bool empty() const {
    return lib::begin(self()) == lib::end(self());
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_sized_range<T>())>
  bool empty() {
    return lib::size(self()) == 0;
  }

  template<typename T = Self, HALCHECK_REQUIRE(!lib::is_sized_range<T>() && lib::is_forward_range<T>())>
  bool empty() {
    return lib::begin(self()) == lib::end(self());
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_sized_range<const T>() || lib::is_forward_range<const T>())>
  constexpr explicit operator bool() const {
    return !empty();
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_sized_range<T>() || lib::is_forward_range<T>())>
  explicit operator bool() {
    return !empty();
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_random_access_range<const T>())>
  constexpr auto size() const -> decltype(lib::to_unsigned(lib::end(self<T>()) - lib::begin(self<T>()))) {
    return lib::to_unsigned(lib::end(self()) - lib::begin(self()));
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_random_access_range<T>())>
  auto size() -> decltype(lib::to_unsigned(lib::end(self<T>()) - lib::begin(self<T>()))) {
    return lib::to_unsigned(lib::end(self()) - lib::begin(self()));
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_forward_range<const T>())>
  constexpr auto front() const -> decltype(*lib::begin(self<T>())) {
    return *lib::begin(self());
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_forward_range<T>())>
  auto front() -> decltype(*lib::begin(self<T>())) {
    return *lib::begin(self());
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_bidirectional_range<const T>())>
  constexpr auto back() const -> decltype(*std::prev(lib::end(self<T>()))) {
    return *std::prev(lib::end(self()));
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_bidirectional_range<T>())>
  auto back() -> decltype(*std::prev(lib::end(self<T>()))) {
    return *std::prev(lib::end(self()));
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_random_access_range<const T>())>
  constexpr auto operator[](lib::range_difference_t<T> n) const -> decltype(lib::begin(self<T>())[n]) {
    return lib::begin(self<T>())[n];
  }

  template<typename T = Self, HALCHECK_REQUIRE(lib::is_random_access_range<T>())>
  auto operator[](lib::range_difference_t<T> n) -> decltype(lib::begin(self<T>())[n]) {
    return lib::begin(self<T>())[n];
  }
};

}} // namespace halcheck::lib

#endif
