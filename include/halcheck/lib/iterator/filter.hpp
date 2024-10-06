#ifndef HALCHECK_LIB_ITERATOR_FILTER_HPP
#define HALCHECK_LIB_ITERATOR_FILTER_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/filter_view/iterator

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
    HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<F &, lib::iter_reference_t<I>>>())>
class filter_iterator : private lib::iterator_interface<filter_iterator<I, F>> {
private:
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE_(lib::is_input_iterator<J>()),
      HALCHECK_REQUIRE_(std::is_copy_constructible<G>()),
      HALCHECK_REQUIRE_(std::is_object<G>()),
      HALCHECK_REQUIRE_(lib::is_boolean_testable<lib::invoke_result_t<G &, lib::iter_reference_t<J>>>())>
  friend class filter_iterator;
  friend class lib::iterator_interface<filter_iterator<I, F>>;

public:
  using lib::iterator_interface<filter_iterator<I, F>>::operator++;
  using lib::iterator_interface<filter_iterator<I, F>>::operator--;

  using reference = lib::iter_reference_t<I>;
  using iterator_category =
      lib::conditional_t<lib::is_random_access_iterator<I>{}, std::bidirectional_iterator_tag, lib::iter_category_t<I>>;
  using value_type = lib::iter_value_t<I>;
  using difference_type = lib::iter_difference_t<I>;
  using pointer = void;

  constexpr filter_iterator() = default;

  filter_iterator(I base, I end, F fun) : _base(std::move(base)), _end(std::move(end)), _func(std::move(fun)) {
    while (_base != _end && !_func(*_base))
      ++_base;
  }

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_convertible<I2, I>()),
      HALCHECK_REQUIRE(std::is_convertible<F2, F>())>
  constexpr filter_iterator(filter_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _end(std::move(other._end)), _func(std::move(*other._func)) {}

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_constructible<I, I2>()),
      HALCHECK_REQUIRE(std::is_constructible<F, F2>()),
      HALCHECK_REQUIRE(!std::is_convertible<I2, I>() || !std::is_convertible<F2, F>())>
  explicit constexpr filter_iterator(filter_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _end(std::move(other._end)), _func(*std::move(other._func)) {}

  constexpr const I &base() const & noexcept { return _base; }

  I base() && { return std::move(_base); }

  constexpr reference operator*() const noexcept(noexcept(*_base)) { return *_base; }

  filter_iterator &operator++() {
    do {
      ++_base;
    } while (_base != _end && !_func(*_base));
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<I>() && _)>
  filter_iterator &operator--() {
    do {
      --_base;
    } while (!_func(*_base));
    return *this;
  }

private:
  friend constexpr bool operator==(const filter_iterator &x, const filter_iterator &y) { return x._base == y._base; }

  I _base, _end;
  lib::assignable<F> _func;
};

static const struct {
  template<typename I, typename F>
  lib::filter_iterator<I, F> operator()(I base, I end, F func) const {
    return lib::filter_iterator<I, F>(std::move(base), std::move(end), std::move(func));
  }
} make_filter_iterator;

}} // namespace halcheck::lib

#endif
