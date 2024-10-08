#ifndef HALCHECK_LIB_ITERATOR_INDEX_HPP
#define HALCHECK_LIB_ITERATOR_INDEX_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

template<typename R, HALCHECK_REQUIRE(lib::is_random_access_range<R>())>
class index_iterator : private lib::iterator_interface<index_iterator<R>> {
public:
  template<typename T, HALCHECK_REQUIRE_(lib::is_random_access_range<T>())>
  friend class index_iterator;

  using lib::iterator_interface<index_iterator>::operator++;
  using lib::iterator_interface<index_iterator>::operator--;
  using lib::iterator_interface<index_iterator>::operator-=;
  using lib::iterator_interface<index_iterator>::operator[];

  using value_type = lib::range_value_t<R>;
  using reference = lib::range_reference_t<R>;
  using pointer = void;
  using difference_type = lib::range_difference_t<R>;
  using iterator_category = std::random_access_iterator_tag;

  index_iterator() = default;

  explicit index_iterator(R &base, difference_type index = 0) : _base(std::addressof(base)), _index(index) {}

  template<typename T, HALCHECK_REQUIRE(std::is_convertible<T *, R *>())>
  index_iterator(const index_iterator<T> &other) // NOLINT: implicit conversion
      : _base(other._base), _index(other._index) {}

  reference operator*() const { return lib::begin(*_base)[_index]; }

  index_iterator &operator++() {
    ++_index;
    return *this;
  }

  index_iterator &operator--() {
    --_index;
    return *this;
  }

  index_iterator &operator+=(difference_type n) {
    _index += n;
    return *this;
  }

  difference_type index() const { return _index; }

private:
  friend bool operator==(const index_iterator &lhs, const index_iterator &rhs) { return lhs._index == rhs._index; }

  friend difference_type operator-(const index_iterator &lhs, const index_iterator &rhs) {
    return lhs._index - rhs._index;
  }

  R *_base = nullptr;
  difference_type _index = 0;
};

}} // namespace halcheck::lib

#endif
