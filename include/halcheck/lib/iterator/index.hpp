#ifndef HALCHECK_LIB_ITERATOR_INDEX_HPP
#define HALCHECK_LIB_ITERATOR_INDEX_HPP

// IWYU pragma: private, include "../iterator.hpp"

#include <halcheck/lib/iterator/ranges.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <iterator>

namespace halcheck { namespace lib {

/// @brief Provides stable iterator access to a random-access container. In
///        particular, iterators remain valid as long as the index to which they
///        point contains the same data.
/// @tparam T The type of container to access.
template<
    typename T,
    HALCHECK_REQUIRE(lib::is_range<T>()),
    HALCHECK_REQUIRE(lib::is_random_access_iterator<lib::iterator_t<T>>())>
class index_iterator {
public:
  template<
      typename U,
      HALCHECK_REQUIRE_(lib::is_range<U>()),
      HALCHECK_REQUIRE_(lib::is_random_access_iterator<lib::iterator_t<U>>())>
  friend class index_iterator;

  using difference_type =
      typename std::common_type<std::ptrdiff_t, typename std::make_signed<lib::range_size_t<T>>::type>::type;
  using reference = decltype(std::declval<T &>()[std::declval<lib::range_size_t<T>>()]);
  using value_type = lib::remove_cv_t<lib::remove_reference_t<reference>>;
  using pointer = lib::remove_reference_t<reference> *;
  using iterator_category = std::random_access_iterator_tag;

  index_iterator() : _owner(nullptr), _index(0) {}

  index_iterator(T &owner, lib::range_size_t<T> index) : _owner(&owner), _index(index) {}

  template<typename U, HALCHECK_REQUIRE(std::is_convertible<U *, T *>())>
  index_iterator(index_iterator<U> other) // NOLINT: implicit conversion
      : _owner(other._owner), _index(other._index) {}

  reference operator*() const { return (*_owner)[_index]; }

  pointer operator->() const { return &**this; }

  index_iterator &operator++() {
    ++_index;
    return *this;
  }

  index_iterator operator++(int) {
    auto output = *this;
    ++_index;
    return output;
  }

  index_iterator &operator+=(difference_type n) {
    _index += n;
    return *this;
  }

  index_iterator &operator-=(difference_type n) {
    _index -= n;
    return *this;
  }

  index_iterator &operator--() {
    --_index;
    return *this;
  }

  index_iterator operator--(int) {
    auto output = *this;
    --_index;
    return output;
  }

  reference operator[](difference_type n) const { return *(*this + n); }

  friend bool operator==(const index_iterator &lhs, const index_iterator &rhs) {
    return lhs._owner == rhs._owner && lhs._index == rhs._index;
  }

  friend bool operator!=(const index_iterator &lhs, const index_iterator &rhs) { return !(lhs == rhs); }

  friend bool operator<(const index_iterator &lhs, const index_iterator &rhs) {
    return lhs._owner < rhs._owner || (lhs._owner == rhs._owner && lhs._index < rhs._index);
  }

  friend bool operator>(const index_iterator &lhs, const index_iterator &rhs) {
    return lhs._owner > rhs._owner || (lhs._owner == rhs._owner && lhs._index > rhs._index);
  }

  friend bool operator<=(const index_iterator &lhs, const index_iterator &rhs) { return !(lhs > rhs); }

  friend bool operator>=(const index_iterator &lhs, const index_iterator &rhs) { return !(lhs < rhs); }

  friend index_iterator operator+(index_iterator lhs, difference_type rhs) { return lhs += rhs; }

  friend index_iterator operator+(difference_type lhs, index_iterator rhs) { return rhs += lhs; }

  friend index_iterator operator-(index_iterator lhs, difference_type rhs) { return lhs -= rhs; }

  friend difference_type operator-(index_iterator lhs, index_iterator rhs) { return lhs._index - rhs._index; }

  T &owner() const { return *_owner; }

  lib::range_size_t<T> index() const { return _index; }

private:
  T *_owner;
  lib::range_size_t<T> _index;
};

template<typename T>
index_iterator<T> make_index_iterator(T &value, lib::range_size_t<T> index) {
  return index_iterator<T>(value, index);
}

}} // namespace halcheck::lib

#endif
