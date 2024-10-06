#ifndef HALCHECK_LIB_ITERATOR_CACHE_HPP
#define HALCHECK_LIB_ITERATOR_CACHE_HPP

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <memory>

namespace halcheck { namespace lib {

template<typename I, HALCHECK_REQUIRE(lib::is_input_iterator<I>())>
class cache_iterator : private lib::iterator_interface<cache_iterator<I>> {
public:
  using iterator_category = lib::iter_category_t<I>;
  using value_type = lib::iter_value_t<I>;
  using reference = const value_type &;
  using pointer = const value_type *;
  using difference_type = lib::iter_difference_t<I>;

  using lib::iterator_interface<cache_iterator<I>>::operator++;
  using lib::iterator_interface<cache_iterator<I>>::operator--;
  using lib::iterator_interface<cache_iterator<I>>::operator[];

  cache_iterator() = default;

  explicit cache_iterator(I base) : _base(std::move(base)) {}

  template<typename J, HALCHECK_REQUIRE(std::is_convertible<J, I>())>
  cache_iterator(cache_iterator<J> other) // NOLINT: implicit conversion
      : _base(std::move(other._base)) {}

  template<typename J, HALCHECK_REQUIRE(!std::is_convertible<J, I>()), HALCHECK_REQUIRE(std::is_constructible<I, J>())>
  explicit cache_iterator(cache_iterator<J> other) // NOLINT: implicit conversion
      : _base(std::move(other._base)) {}

  const I &base() const & { return _base; }
  I &&base() && { return std::move(_base); }

  reference operator*() const {
    if (!_value)
      _value.emplace(*_base);

    return *_value;
  }

  pointer operator->() const { return std::addressof(**this); }

  cache_iterator &operator++() {
    _value.reset();
    ++_base;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<I>() && _)>
  cache_iterator &operator--() {
    _value.reset();
    --_base;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  cache_iterator &operator+=(difference_type n) {
    _value.reset();
    _base += n;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  cache_iterator &operator-=(difference_type n) {
    _value.reset();
    _base -= n;
    return *this;
  }

  friend bool operator==(const cache_iterator &lhs, const cache_iterator &rhs) { return lhs._base == rhs._base; }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend bool operator<(const cache_iterator &lhs, const cache_iterator &rhs) {
    return lhs._base < rhs._base;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend difference_type operator-(const cache_iterator &lhs, const cache_iterator &rhs) {
    return lhs._base - rhs._base;
  }

private:
  I _base;
  mutable lib::optional<lib::iter_value_t<I>> _value;
};

template<typename I>
lib::cache_iterator<I> make_cache_iterator(I base) {
  return lib::cache_iterator<I>(std::move(base));
}

}} // namespace halcheck::lib

#endif
