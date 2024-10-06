#ifndef HALCHECK_LIB_ITERATOR_JOIN_HPP
#define HALCHECK_LIB_ITERATOR_JOIN_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

template<
    typename I,
    HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
    HALCHECK_REQUIRE(std::is_reference<lib::iter_reference_t<I>>()),
    HALCHECK_REQUIRE(lib::is_input_range<lib::iter_reference_t<I>>())>
class join_iterator : private lib::iterator_interface<join_iterator<I>> {
public:
  using inner = lib::iterator_t<lib::iter_reference_t<I>>;

  using lib::iterator_interface<join_iterator>::operator++;
  using lib::iterator_interface<join_iterator>::operator--;
  using lib::iterator_interface<join_iterator>::operator[];

  using iterator_category = lib::conditional_t<
      lib::is_bidirectional_iterator<I>() && lib::is_bidirectional_iterator<inner>(),
      std::bidirectional_iterator_tag,
      lib::conditional_t<
          lib::is_forward_iterator<I>() && lib::is_forward_iterator<inner>(),
          std::forward_iterator_tag,
          std::input_iterator_tag>>;
  using reference = lib::iter_reference_t<inner>;
  using pointer = void;
  using value_type = lib::range_value_t<lib::iter_reference_t<I>>;
  using difference_type =
      lib::common_type_t<lib::iter_difference_t<I>, lib::range_difference_t<lib::iter_reference_t<I>>>;

private:
  template<
      typename J,
      HALCHECK_REQUIRE_(lib::is_input_iterator<J>()),
      HALCHECK_REQUIRE_(std::is_reference<lib::iter_reference_t<J>>()),
      HALCHECK_REQUIRE_(lib::is_input_range<lib::iter_reference_t<J>>())>
  friend class join_iterator;
  friend struct join_sentinel;
  friend class lib::iterator_interface<join_iterator>;

  I _begin;
  I _end;
  lib::optional<inner> _inner;

  void satisfy() {
    for (; _begin != _end; ++_begin) {
      auto &&inner = *_begin;
      _inner.emplace(lib::begin(inner));
      if (*_inner != lib::end(inner))
        return;
    }

    _inner.reset();
  }

public:
  constexpr join_iterator() = default;

  join_iterator(I begin, I end) : _begin(std::move(begin)), _end(std::move(end)) { satisfy(); }

  constexpr auto operator*() const noexcept(noexcept(**this->_inner)) -> decltype(**this->_inner) { return **_inner; }

  template<typename T = inner, typename = decltype(&T::operator->), HALCHECK_REQUIRE(lib::is_copyable<T>())>
  constexpr inner operator->() const {
    return *_inner;
  }

  join_iterator &operator++() {
    auto &&inner_rng = *_begin;
    if (++*_inner == lib::end(inner_rng)) {
      ++_begin;
      satisfy();
    }
    return *this;
  }

  template<
      typename T = I,
      HALCHECK_REQUIRE(lib::is_bidirectional_iterator<T>()),
      HALCHECK_REQUIRE(lib::is_bidirectional_range<lib::iter_reference_t<T>>())>
  join_iterator &operator--() {
    if (_begin == _end)
      _inner.emplace(lib::end(*--_begin));

    while (*_inner == lib::begin(*_begin))
      _inner.emplace(lib::end(*--_begin));

    --*_inner;

    return *this;
  }

private:
  template<
      bool _ = true,
      HALCHECK_REQUIRE(lib::is_equality_comparable<I>() && _),
      HALCHECK_REQUIRE(lib::is_equality_comparable<inner>() && _)>
  friend constexpr bool operator==(const join_iterator &x, const join_iterator &y) {
    return x._begin == y._begin && x._end == y._end && x._inner == y._inner;
  }
};

static const struct {
  template<typename I>
  lib::join_iterator<I> operator()(I begin, I end) const {
    return lib::join_iterator<I>(std::move(begin), std::move(end));
  }
} make_join_iterator;

template<
    typename V,
    HALCHECK_REQUIRE(lib::is_view<V>()),
    HALCHECK_REQUIRE(lib::is_input_range<V>()),
    HALCHECK_REQUIRE(std::is_reference<lib::range_reference_t<V>>()),
    HALCHECK_REQUIRE(lib::is_input_range<lib::range_reference_t<V>>())>
class join_view : public lib::view_interface<join_view<V>> {
public:
  join_view() = default;

  explicit join_view(V base) : _base(std::move(base)) {}

  template<bool _ = true, HALCHECK_REQUIRE(std::is_copy_constructible<V>() && _)>
  constexpr V base() const & {
    return _base;
  }

  V base() && { return std::move(_base); }

  lib::join_iterator<lib::iterator_t<V>> begin() { return lib::make_join_iterator(lib::begin(_base), lib::end(_base)); }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_input_range<const U>()),
      HALCHECK_REQUIRE(std::is_reference<lib::range_reference_t<const U>>())>
  lib::join_iterator<lib::iterator_t<const U>> begin() const {
    return lib::make_join_iterator(lib::begin(_base), lib::end(_base));
  }

  lib::join_iterator<lib::iterator_t<V>> end() { return lib::make_join_iterator(lib::end(_base), lib::end(_base)); }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_input_range<const U>()),
      HALCHECK_REQUIRE(std::is_reference<lib::range_reference_t<const U>>())>
  lib::join_iterator<lib::iterator_t<const U>> end() const {
    return lib::make_join_iterator(lib::end(_base), lib::end(_base));
  }

private:
  V _base;
};

static const struct {
  template<typename V>
  lib::join_view<V> operator()(V base) const {
    return lib::join_view<V>(std::move(base));
  }
} join;

}} // namespace halcheck::lib

#endif
