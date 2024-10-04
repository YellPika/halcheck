#ifndef HALCHECK_LIB_ITERATOR_CACHE_HPP
#define HALCHECK_LIB_ITERATOR_CACHE_HPP

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

template<typename I>
class cache_iterator : private lib::iterator_interface<cache_iterator<I>> {
public:
  using iterator_concept = lib::iter_concept_t<I>;
  // using iterator_category = TODO
  using value_type = lib::iter_value_t<I>;
  using difference_type = lib::iter_difference_t<I>;

  using lib::iterator_interface<cache_iterator<I>>::operator++;
  using lib::iterator_interface<cache_iterator<I>>::operator--;
  using lib::iterator_interface<cache_iterator<I>>::operator[];

  cache_iterator() = default;

  explicit cache_iterator(I base) : _base(std::move(base)) {}

  const I &base() const & { return _base; }
  I &&base() && { return std::move(_base); }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::dereference, J>())>
  const lib::iter_value_t<J> &operator*() const {
    if (!_value)
      _value.emplace(*_base);

    return *_value;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_detected<detail::increment, I>())>
  cache_iterator &operator++() {
    _value.reset();
    ++_base;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_detected<detail::decrement, I>())>
  cache_iterator &operator--() {
    _value.reset();
    --_base;
    return *this;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::addition, J>())>
  cache_iterator &operator+=(lib::iter_difference_t<J> n) {
    _value.reset();
    _base += n;
    return *this;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_detected<detail::subtraction, J>())>
  cache_iterator &operator-=(lib::iter_difference_t<J> n) {
    _value.reset();
    _base -= n;
    return *this;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_detected<detail::equality, I>())>
  friend bool operator==(const cache_iterator &lhs, const cache_iterator &rhs) {
    return lhs._base == rhs._base;
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_detected<detail::equality, I>())>
  friend bool operator<(const cache_iterator &lhs, const cache_iterator &rhs) {
    return lhs._base < rhs._base;
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
