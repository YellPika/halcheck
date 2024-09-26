#ifndef HALCHECK_LIB_ITERATOR_CONCAT_HPP
#define HALCHECK_LIB_ITERATOR_CONCAT_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/ranges.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>()), HALCHECK_REQUIRE(lib::is_range<lib::iter_value_t<I>>())>
struct concat_iterator {
public:
  using difference_type = lib::range_difference_t<lib::iter_value_t<I>>;
  using reference = lib::range_reference_t<lib::iter_value_t<I>>;
  using value_type = lib::range_value_t<lib::iter_value_t<I>>;
  using pointer = lib::range_pointer_t<lib::iter_value_t<I>>;
  using iterator_category = lib::range_category_t<lib::iter_value_t<I>>;

  concat_iterator() = default;

  concat_iterator(I begin, I end) {
    if (begin != end)
      _data = data(std::move(begin), std::move(end));
  }

  template<
      typename J,
      HALCHECK_REQUIRE(std::is_convertible<J, I>()),
      HALCHECK_REQUIRE(
          std::is_convertible<lib::iterator_t<lib::iter_value_t<J>>, lib::iterator_t<lib::iter_value_t<I>>>())>
  concat_iterator(concat_iterator<J> other) // NOLINT: implicit conversion
      : _data(std::move(other._data)) {}

  reference operator*() const { return *_data->inner_begin; }

  pointer operator->() const { return &*_data->inner_begin; }

  concat_iterator &operator++() {
    ++_data->inner_begin;
    while (_data && _data->inner_begin == _data->inner_end) {
      if (++_data->outer_begin == _data->outer_end)
        _data.reset();
      else {
        _data->inner_begin = lib::begin(*_data->outer_begin);
        _data->inner_end = lib::end(*_data->outer_begin);
      }
    }

    return *this;
  }

  concat_iterator operator++(int) {
    auto output = *this;
    ++*this;
    return output;
  }

  friend bool operator==(const concat_iterator &lhs, const concat_iterator &rhs) { return lhs._data == rhs._data; }

  friend bool operator!=(const concat_iterator &lhs, const concat_iterator &rhs) { return !(lhs == rhs); }

private:
  struct data {
    I outer_begin, outer_end;
    lib::iterator_t<lib::iter_value_t<I>> inner_begin, inner_end;

    data(I begin, I end)
        : outer_begin(std::move(begin)), outer_end(std::move(end)), inner_begin(lib::begin(*outer_begin)),
          inner_end(lib::end(*outer_begin)) {}

    template<
        typename J,
        HALCHECK_REQUIRE(std::is_convertible<J, I>()),
        HALCHECK_REQUIRE(
            std::is_convertible<lib::iterator_t<lib::iter_value_t<J>>, lib::iterator_t<lib::iter_value_t<I>>>())>
    data(typename concat_iterator<J>::data other) // NOLINT: implicit conversion
        : outer_begin(std::move(other.outer_begin)), outer_end(std::move(other.outer_end)),
          inner_begin(std::move(other.inner_begin)), inner_end(std::move(other.inner_end)) {}

    bool operator==(const data &other) const {
      return outer_begin == other.outer_begin && outer_end == other.outer_end && inner_begin == other.inner_begin &&
             inner_end == other.inner_end;
    }
  };

  lib::optional<data> _data;
};

template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>()), HALCHECK_REQUIRE(lib::is_range<lib::iter_value_t<I>>())>
lib::concat_iterator<I> make_concat_iterator(I begin, I end) {
  return lib::concat_iterator<I>(std::move(begin), std::move(end));
}

template<typename R, HALCHECK_REQUIRE(lib::is_range<R>()), HALCHECK_REQUIRE(lib::is_range<lib::range_value_t<R>>())>
using concat_view = lib::view<lib::concat_iterator<lib::iterator_t<R>>>;

template<typename R, HALCHECK_REQUIRE(lib::is_range<R>()), HALCHECK_REQUIRE(lib::is_range<lib::range_value_t<R>>())>
lib::concat_view<lib::remove_cv_t<R>> make_concat_view(R &&range) {
  return lib::make_view(
      lib::make_concat_iterator(lib::begin(range), lib::end(range)),
      lib::make_concat_iterator(lib::end(range), lib::end(range)));
}

}} // namespace halcheck::lib

#endif
