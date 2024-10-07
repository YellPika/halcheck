#ifndef HALCHECK_LIB_ITERATOR_FILTER_HPP
#define HALCHECK_LIB_ITERATOR_FILTER_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <memory>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/filter_view/iterator

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
    HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::iter_reference_t<I>>>())>
class filter_iterator : private lib::iterator_interface<filter_iterator<I, F>> {
private:
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE_(lib::is_input_iterator<J>()),
      HALCHECK_REQUIRE_(std::is_copy_constructible<G>()),
      HALCHECK_REQUIRE_(std::is_object<G>()),
      HALCHECK_REQUIRE_(lib::is_boolean_testable<lib::invoke_result_t<const G &, lib::iter_reference_t<J>>>())>
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

  constexpr reference operator*() const noexcept(noexcept(*std::declval<const I &>())) { return *_base; }

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

template<
    typename V,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_range<V>()),
    HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<V>>>())>
class filter_view : public lib::view_interface<filter_view<V, F>> {
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
  filter_view() = default;

  filter_view(V base, F func) : _base(std::move(base)), _func(std::move(func)) {}

  template<bool _ = true, HALCHECK_REQUIRE(std::is_copy_constructible<V>() && _)>
  constexpr V base() const & {
    return _base;
  }

  V base() && { return std::move(_base); }

  lib::filter_iterator<lib::iterator_t<V>, ref> begin() {
    return lib::make_filter_iterator(lib::begin(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_input_range<const U>()),
      HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<const U>>>())>
  lib::filter_iterator<lib::iterator_t<const U>, ref> begin() const {
    return lib::make_filter_iterator(lib::begin(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  lib::filter_iterator<lib::iterator_t<V>, ref> end() {
    return lib::make_filter_iterator(lib::end(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<const U>>>())>
  lib::filter_iterator<lib::iterator_t<const U>, ref> end() const {
    return lib::make_filter_iterator(lib::end(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

private:
  V _base;
  lib::assignable<F> _func;
};

template<typename V, typename F>
lib::filter_view<V, F> filter(V base, F func) {
  return lib::filter_view<V, F>(std::move(base), std::move(func));
}

}} // namespace halcheck::lib

#endif
