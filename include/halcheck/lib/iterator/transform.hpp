#ifndef HALCHECK_LIB_ITERATOR_TRANSFORM_HPP
#define HALCHECK_LIB_ITERATOR_TRANSFORM_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/transform_view/iterator

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
    HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::iter_reference_t<I>>())>
class transform_iterator : private lib::iterator_interface<transform_iterator<I, F>> {
private:
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE_(lib::is_input_iterator<J>()),
      HALCHECK_REQUIRE_(std::is_copy_constructible<G>()),
      HALCHECK_REQUIRE_(std::is_object<G>()),
      HALCHECK_REQUIRE_(lib::is_invocable<const G &, lib::iter_reference_t<J>>())>
  friend class transform_iterator;
  friend class lib::iterator_interface<transform_iterator<I, F>>;

public:
  using lib::iterator_interface<transform_iterator<I, F>>::operator++;
  using lib::iterator_interface<transform_iterator<I, F>>::operator--;
  using lib::iterator_interface<transform_iterator<I, F>>::operator-=;
  using lib::iterator_interface<transform_iterator<I, F>>::operator[];

  using reference = lib::invoke_result_t<const F &, lib::iter_reference_t<I>>;
  using iterator_category =
      lib::conditional_t<!std::is_reference<reference>(), std::input_iterator_tag, lib::iter_category_t<I>>;
  using value_type = lib::remove_cvref_t<lib::invoke_result_t<const F &, lib::iter_reference_t<I>>>;
  using difference_type = lib::iter_difference_t<I>;
  using pointer = void;

  constexpr transform_iterator() = default;

  constexpr transform_iterator(I current, F fun) : _base(std::move(current)), _func(std::move(fun)) {}

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_convertible<I2, I>()),
      HALCHECK_REQUIRE(std::is_convertible<F2, F>())>
  constexpr transform_iterator(transform_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _func(std::move(*other._func)) {}

  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_constructible<I, I2>()),
      HALCHECK_REQUIRE(std::is_constructible<F, F2>()),
      HALCHECK_REQUIRE(!std::is_convertible<I2, I>() || !std::is_convertible<F2, F>())>
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

private:
  friend constexpr bool operator==(const transform_iterator &x, const transform_iterator &y) {
    return x._base == y._base;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend difference_type operator-(const transform_iterator &x, const transform_iterator &y) {
    return x._base - y._base;
  }

  I _base;
  lib::assignable<F> _func;
};

static const struct {
  template<
      typename I,
      typename F,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
      HALCHECK_REQUIRE(std::is_object<F>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::iter_reference_t<I>>())>
  lib::transform_iterator<I, F> operator()(I base, F func) const {
    return lib::transform_iterator<I, F>(std::move(base), std::move(func));
  }
} make_transform_iterator;

template<
    typename V,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_range<V>()),
    HALCHECK_REQUIRE(std::is_move_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<V>>())>
class transform_view : public lib::view_interface<transform_view<V, F>> {
private:
  struct ref {
    explicit ref(const F *base = nullptr) : base(base) {}
    template<typename... Args>
    lib::invoke_result_t<const F &, Args...> operator()(Args &&...args) const {
      return lib::invoke(*base, std::forward<Args>(args)...);
    }
    const F *base;
  };

public:
  constexpr transform_view() = default;

  constexpr transform_view(V base, F func) : _base(std::move(base)), _func(std::move(func)) {}

  template<bool _ = true, HALCHECK_REQUIRE(std::is_copy_constructible<V>() && _)>
  constexpr V base() const & {
    return _base;
  }

  V base() && { return std::move(_base); }

  lib::transform_iterator<lib::iterator_t<V>, ref> begin() {
    return lib::make_transform_iterator(lib::begin(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<const U>>())>
  lib::transform_iterator<lib::iterator_t<const U>, ref> begin() const {
    return lib::make_transform_iterator(lib::begin(_base), ref{std::addressof(*_func)});
  }

  lib::transform_iterator<lib::iterator_t<V>, ref> end() {
    return lib::make_transform_iterator(lib::end(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<const U>>())>
  lib::transform_iterator<lib::iterator_t<const U>, ref> end() const {
    return lib::make_transform_iterator(lib::end(_base), ref{std::addressof(*_func)});
  }

  template<typename U = V, HALCHECK_REQUIRE(lib::is_sized_range<U>())>
  lib::range_difference_t<U> size() {
    return lib::size(_base);
  }

  template<typename U = V, HALCHECK_REQUIRE(lib::is_sized_range<const U>())>
  lib::range_difference_t<const U> size() const {
    return lib::size(_base);
  }

private:
  V _base;
  lib::assignable<F> _func;
};

template<
    typename V,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_range<V>()),
    HALCHECK_REQUIRE(std::is_move_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<V>>())>
lib::transform_view<V, F> transform(V base, F func) {
  return lib::transform_view<V, F>(std::move(base), std::move(func));
}

}} // namespace halcheck::lib

#endif
