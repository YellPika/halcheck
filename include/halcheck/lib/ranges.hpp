#ifndef HALCHECK_LIB_RANGES_HPP
#define HALCHECK_LIB_RANGES_HPP

#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <type_traits>

#if __cplusplus >= 202002L
#include <ranges>
#endif

namespace halcheck { namespace lib {

#if __cplusplus >= 202002L
using std::ranges::begin;
using std::ranges::end;
using std::ranges::iterator_t;
using std::ranges::range_reference_t;
using std::ranges::range_value_t;
using std::ranges::sentinel_t;
#else
namespace detail {
using std::begin;
using std::end;

template<typename T>
using iterator_t = decltype(begin(std::declval<T &>()));

template<typename T>
using sentinel_t = decltype(end(std::declval<T &>()));
} // namespace detail

using detail::iterator_t;
using detail::sentinel_t;

static constexpr struct {
  template<typename T>
  iterator_t<T> operator()(T &&value) const {
    using std::begin;
    return begin(value);
  }
} begin;

static constexpr struct {
  template<typename T>
  sentinel_t<T> operator()(T &&value) const {
    using std::end;
    return end(value);
  }
} end;

template<typename T>
using range_value_t = typename std::iterator_traits<iterator_t<T>>::value_type;

template<typename T>
using range_reference_t = typename std::iterator_traits<iterator_t<T>>::reference;
#endif

/// @brief Tests if a type T supports begin(T) and end(T).
/// @tparam T The type to perform the test on.
template<typename T>
struct is_range : lib::conjunction<lib::is_detected<iterator_t, T>, lib::is_detected<sentinel_t, T>> {};

namespace detail {
template<typename T>
using is_insertable_helper =
    decltype(std::declval<T &>().insert(lib::begin(std::declval<T &>()), std::declval<lib::range_value_t<T>>()));
} // namespace detail

/// @brief Tests if a range can be inserted into.
/// @tparam T The type to perform the test on.
template<typename T>
struct is_insertable : lib::is_detected<detail::is_insertable_helper, T> {};

namespace detail {
template<typename T>
using is_iterator_helper = typename std::iterator_traits<T>::iterator_category;
} // namespace detail

/// @brief Tests if a type is an iterator type.
/// @tparam T The type to perform the test on.
template<typename T>
struct is_iterator : lib::is_detected<detail::is_iterator_helper, T> {};

template<typename T>
class auto_insert_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = typename std::iterator_traits<lib::iterator_t<T>>::value_type;
  using difference_type = typename std::iterator_traits<lib::iterator_t<T>>::difference_type;
  using pointer = typename std::iterator_traits<lib::iterator_t<T>>::pointer;
  using reference = typename std::iterator_traits<lib::iterator_t<T>>::reference;
  using container_type = T;

  explicit auto_insert_iterator(T &container) : auto_insert_iterator(container, lib::begin(container)) {}

  auto_insert_iterator(T &container, lib::iterator_t<T> it) : _container(&container), _it(std::move(it)) {}

  typename std::iterator_traits<lib::iterator_t<T>>::reference operator*() {
    reserve();
    return *_it;
  }
  typename std::iterator_traits<lib::iterator_t<T>>::pointer operator->() {
    reserve();
    return _it.operator->();
  }

  auto_insert_iterator &operator++() {
    reserve();
    ++_it;
    return *this;
  }

  auto_insert_iterator operator++(int) { return reserve(*_container, _it++); }

  lib::iterator_t<T> inner() const { return _it; }

private:
  void reserve() {
    if (_it == lib::end(*_container))
      _it = _container->emplace(_it);
  }

  T *_container;
  lib::iterator_t<T> _it;
};

template<typename T>
auto_insert_iterator<T> auto_insert(T &container) {
  return auto_insert_iterator<T>(container);
}

template<typename T>
auto_insert_iterator<T> auto_insert(T &container, lib::iterator_t<T> it) {
  return auto_insert_iterator<T>(container, it);
}

}} // namespace halcheck::lib

#endif
