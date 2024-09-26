#ifndef HALCHECK_LIB_ITERATOR_TRANSFORM_HPP
#define HALCHECK_LIB_ITERATOR_TRANSFORM_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/ranges.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <type_traits>

namespace halcheck { namespace lib {

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_iterator<I>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, lib::iter_reference_t<I>>())>
class transform_iterator {
public:
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE_(lib::is_iterator<J>()),
      HALCHECK_REQUIRE_(lib::is_invocable<G, lib::iter_reference_t<J>>())>
  friend class transform_iterator;

  using difference_type = lib::iter_difference_t<I>;
  using reference = lib::invoke_result_t<F, lib::iter_reference_t<I>>;
  using value_type = lib::decay_t<reference>;
  using pointer = void;
  using iterator_category = std::input_iterator_tag;

  template<
      typename J = I,
      typename G = F,
      HALCHECK_REQUIRE(std::is_default_constructible<J>()),
      HALCHECK_REQUIRE(std::is_default_constructible<G>())>
  transform_iterator() : _impl(), _func() {}

  transform_iterator(I impl, F func) : _impl(std::move(impl)), _func(std::move(func)) {}

  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE(std::is_convertible<J, I>()),
      HALCHECK_REQUIRE(std::is_convertible<G, F>())>
  transform_iterator(transform_iterator<J, G> other) // NOLINT: implicit conversion
      : _impl(std::move(other._impl)), _func(std::move(other._func)) {}

  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE(std::is_constructible<I, J>()),
      HALCHECK_REQUIRE(std::is_constructible<F, G>()),
      HALCHECK_REQUIRE(!std::is_convertible<J, I>() || !std::is_convertible<G, F>())>
  explicit transform_iterator(transform_iterator<J, G> other)
      : _impl(std::move(other._impl)), _func(std::move(other._func)) {}

  template<typename J = I, HALCHECK_REQUIRE(lib::is_input_iterator<J>())>
  bool operator==(const transform_iterator &other) const {
    return _impl == other._impl;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_input_iterator<J>())>
  bool operator!=(const transform_iterator &other) const {
    return _impl != other._impl;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  bool operator<(const transform_iterator &other) const {
    return _impl < other._impl;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  bool operator>(const transform_iterator &other) const {
    return _impl > other._impl;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  bool operator<=(const transform_iterator &other) const {
    return _impl <= other._impl;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  bool operator>=(const transform_iterator &other) const {
    return _impl >= other._impl;
  }

  reference operator*() const { return _func(*_impl); }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  reference operator[](difference_type n) const {
    return _func(_impl[n]);
  }

  transform_iterator &operator++() {
    ++_impl;
    return *this;
  }

  transform_iterator<decltype(std::declval<I &>()++), F> operator++(int) {
    return transform_iterator<decltype(std::declval<I &>()++), F>(_impl++, _func);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<J>())>
  transform_iterator &operator--() {
    --_impl;
    return *this;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<J>())>
  transform_iterator operator--(int) {
    return transform_iterator(_impl--, _func);
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  transform_iterator &operator+=(difference_type n) {
    _impl += n;
    return *this;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  transform_iterator &operator-=(difference_type n) {
    _impl -= n;
    return *this;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  friend transform_iterator operator+(transform_iterator lhs, difference_type rhs) {
    return lhs += rhs;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  friend transform_iterator operator+(difference_type lhs, transform_iterator rhs) {
    return rhs += lhs;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  friend transform_iterator operator-(transform_iterator lhs, difference_type rhs) {
    return lhs -= rhs;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  friend difference_type operator-(const transform_iterator &lhs, const transform_iterator &rhs) {
    return lhs._impl - rhs._impl;
  }

  I base() const { return _impl; }

  F functor() const { return _func; }

private:
  I _impl;
  lib::assignable<F> _func;
};

template<typename I, typename F>
lib::transform_iterator<I, F> make_transform_iterator(I it, F func) {
  return lib::transform_iterator<I, F>(std::move(it), std::move(func));
}

template<
    typename R,
    typename F,
    HALCHECK_REQUIRE(lib::is_range<R>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, lib::range_reference_t<R>>())>
using transform_view = lib::view<lib::transform_iterator<lib::iterator_t<R>, F>>;

template<
    typename R,
    typename F,
    HALCHECK_REQUIRE(lib::is_range<R>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, lib::range_reference_t<R>>())>
lib::transform_view<lib::remove_cv_t<R>, F> make_transform_view(R &&range, F func) {
  return lib::make_view(
      lib::make_transform_iterator(lib::begin(range), func),
      lib::make_transform_iterator(lib::end(range), func));
}

}} // namespace halcheck::lib

#endif
