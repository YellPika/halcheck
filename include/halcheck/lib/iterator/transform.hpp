#ifndef HALCHECK_LIB_ITERATOR_TRANSFORM_HPP
#define HALCHECK_LIB_ITERATOR_TRANSFORM_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/transform_view/iterator

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
    HALCHECK_REQUIRE(lib::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_invocable<F &, lib::iter_reference_t<I>>())>
class transform_iterator : private lib::iterator_interface<transform_iterator<I, F>> {
private:
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE_(lib::is_input_iterator<J>()),
      HALCHECK_REQUIRE_(lib::is_copy_constructible<G>()),
      HALCHECK_REQUIRE_(std::is_object<G>()),
      HALCHECK_REQUIRE_(lib::is_invocable<G &, lib::iter_reference_t<J>>())>
  friend class transform_iterator;
  friend class lib::iterator_interface<transform_iterator<I, F>>;

public:
  using lib::iterator_interface<transform_iterator<I, F>>::operator++;
  using lib::iterator_interface<transform_iterator<I, F>>::operator--;
  using lib::iterator_interface<transform_iterator<I, F>>::operator[];

  using iterator_concept = lib::conditional_t<
      lib::is_random_access_iterator<I>{},
      std::random_access_iterator_tag,
      lib::conditional_t<
          lib::is_bidirectional_iterator<I>{},
          std::bidirectional_iterator_tag,
          lib::conditional_t<lib::is_forward_iterator<I>{}, std::forward_iterator_tag, std::input_iterator_tag>>>;
  using reference = lib::invoke_result_t<const F &, lib::iter_reference_t<I>>;
  using iterator_category =
      lib::conditional_t<!std::is_reference<reference>(), std::input_iterator_tag, lib::iter_category_t<I>>;
  using value_type = lib::remove_cvref_t<lib::invoke_result_t<F &, lib::iter_reference_t<I>>>;
  using difference_type = lib::iter_difference_t<I>;
  using pointer = void;

  constexpr transform_iterator() = default;

  constexpr transform_iterator(I current, F fun) : _base(std::move(current)), _func(std::move(fun)) {}

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(lib::is_convertible_to<I2, I>()),
      HALCHECK_REQUIRE(lib::is_convertible_to<F2, F>())>
  constexpr transform_iterator(transform_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _func(std::move(*other._func)) {}

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(lib::is_constructible_from<I, I2>()),
      HALCHECK_REQUIRE(lib::is_constructible_from<F, F2>()),
      HALCHECK_REQUIRE(!lib::is_convertible_to<I2, I>() || !lib::is_convertible_to<F2, F>())>
  explicit constexpr transform_iterator(transform_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _func(*std::move(other._func)) {}

  constexpr const I &base() const & noexcept { return _base; }

  I base() && { return std::move(_base); }

  constexpr lib::invoke_result_t<const F &, lib::iter_reference_t<I>> operator*() const
      noexcept(noexcept(lib::invoke(this->_func, *this->_base))) {
    return lib::invoke(_func, *_base);
  }

  transform_iterator &operator++() {
    ++_base;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<I>() && _)>
  transform_iterator &operator--() {
    --_base;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  transform_iterator &operator+=(difference_type n) {
    _base += n;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  transform_iterator &operator-=(difference_type n) {
    _base -= n;
    return *this;
  }

private:
  template<bool _ = true, HALCHECK_REQUIRE(lib::is_equality_comparable<I>() && _)>
  friend constexpr bool operator==(const transform_iterator &x, const transform_iterator &y) {
    return x._base == y._base;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend constexpr bool operator<(const transform_iterator &x, const transform_iterator &y) {
    return x._base < y._base;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend difference_type operator-(const transform_iterator &x, const transform_iterator &y) {
    return x._base - y._base;
  }

  I _base;
  lib::assignable<F> _func;
};

static const struct {
  template<typename I, typename F>
  lib::transform_iterator<I, F> operator()(I base, F func) const {
    return lib::transform_iterator<I, F>(std::move(base), std::move(func));
  }
} make_transform_iterator;

}} // namespace halcheck::lib

#endif
