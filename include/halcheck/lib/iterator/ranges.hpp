#ifndef HALCHECK_LIB_ITERATOR_RANGES_HPP
#define HALCHECK_LIB_ITERATOR_RANGES_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <iterator>

namespace halcheck { namespace lib {

namespace detail {
using std::begin;

template<typename T>
using iterator_t = decltype(begin(std::declval<T &>()));

template<typename T>
using const_iterator_t = decltype(begin(std::declval<const T &>()));
} // namespace detail

using detail::const_iterator_t;
using detail::iterator_t;

static constexpr struct {
  template<typename T>
  iterator_t<T> operator()(T &&value) const {
    using std::begin;
    return begin(value);
  }
} begin;

static constexpr struct {
  template<typename T>
  iterator_t<T> operator()(T &&value) const {
    using std::end;
    return end(value);
  }
} end;

namespace detail {
#if __cplusplus >= 201703L
using std::size;
#else
template<typename T>
constexpr auto size(const T &range) -> decltype(range.size()) {
  return range.size();
}

template<typename T, std::size_t N>
constexpr auto size(const T (&)[N]) -> std::size_t { // NOLINT
  return N;
}
#endif

template<typename T>
using range_size_t = decltype(size(std::declval<T &>()));
} // namespace detail

using detail::range_size_t;

static constexpr struct {
  template<typename T>
  lib::range_size_t<T> operator()(T &&value) const {
    using detail::size;
    return size(value);
  }
} size;

template<typename T>
using range_value_t = typename std::iterator_traits<iterator_t<T>>::value_type;

template<typename T>
using range_reference_t = typename std::iterator_traits<iterator_t<T>>::reference;

template<typename T>
using range_difference_t = typename std::iterator_traits<iterator_t<T>>::difference_type;

template<typename T>
using range_pointer_t = typename std::iterator_traits<iterator_t<T>>::pointer;

template<typename T>
using range_category_t = typename std::iterator_traits<iterator_t<T>>::iterator_category;

namespace detail {
template<typename T>
using is_range_helper = lib::enable_if_t<lib::is_iterator<lib::iterator_t<T>>{}>;
} // namespace detail

/// @brief Tests if a type T supports begin(T) and end(T).
/// @tparam T The type to perform the test on.
template<typename T>
struct is_range : lib::is_detected<detail::is_range_helper, T> {};

namespace detail {
template<typename T>
using is_input_range_helper = lib::enable_if_t<lib::is_input_iterator<lib::iterator_t<T>>{}>;
} // namespace detail

template<typename T>
struct is_input_range : lib::is_detected<detail::is_input_range_helper, T> {};

namespace detail {
template<typename T>
using is_forward_range_helper = lib::enable_if_t<lib::is_forward_iterator<lib::iterator_t<T>>{}>;
} // namespace detail

template<typename T>
struct is_forward_range : lib::is_detected<detail::is_forward_range_helper, T> {};

namespace detail {
template<typename T>
using is_bidirectional_range_helper = lib::enable_if_t<lib::is_bidirectional_iterator<lib::iterator_t<T>>{}>;
} // namespace detail

template<typename T>
struct is_bidirectional_range : lib::is_detected<detail::is_bidirectional_range_helper, T> {};

namespace detail {
template<typename T>
using is_random_access_range_helper = lib::enable_if_t<lib::is_random_access_iterator<lib::iterator_t<T>>{}>;
} // namespace detail

template<typename T>
struct is_random_access_range : lib::is_detected<detail::is_random_access_range_helper, T> {};

namespace detail {
template<typename T>
using is_insertable_helper =
    decltype(std::declval<T &>().insert(lib::begin(std::declval<T &>()), std::declval<lib::range_value_t<T>>()));
} // namespace detail

/// @brief Tests if a range can be inserted into.
/// @tparam T The type to perform the test on.
template<typename T>
struct is_insertable : lib::is_detected<detail::is_insertable_helper, T> {};

template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>())>
class view {
public:
  template<typename J, HALCHECK_REQUIRE_(lib::is_iterator<J>())>
  friend class view;

  view(I begin, I end) : _begin(std::move(begin)), _end(std::move(end)) {}

  template<typename J, HALCHECK_REQUIRE(std::is_convertible<J, I>())>
  view(view<J> other) // NOLINT: implicit conversion
      : _begin(std::move(other._begin)), _end(std::move(other._end)) {}

  template<typename J = I, HALCHECK_REQUIRE(lib::is_random_access_iterator<J>())>
  typename std::make_unsigned<lib::iter_difference_t<I>>::type size() const {
    return _end - _begin;
  }

  template<typename J = I, HALCHECK_REQUIRE(lib::is_input_iterator<J>())>
  bool empty() const {
    return _begin == _end;
  }

  I begin() const { return _begin; }
  I end() const { return _end; }

private:
  I _begin, _end;
};

template<typename I>
bool operator==(const view<I> &lhs, const view<I> &rhs) {
  return lhs.begin() == rhs.begin() && lhs.end() == rhs.end();
}

template<typename I>
bool operator!=(const view<I> &lhs, const view<I> &rhs) {
  return !(lhs == rhs);
}

template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>())>
lib::view<I> make_view(I begin, I end) {
  return lib::view<I>(std::move(begin), std::move(end));
}

// template<
//     typename I,
//     typename F,
//     HALCHECK_REQUIRE(lib::is_iterator<I>()),
//     HALCHECK_REQUIRE(lib::is_invocable_r<bool, F>())>
// bool any_permutation(I begin, I end, F func) {
//   if (begin == end)
//     return func();

//   for (auto i = begin; i != end; ++i) {
//     std::swap(*begin, *i);

//     if (any_permutation(begin + 1, end, func))
//       return true;

//     std::swap(*begin, *i);
//   }

//   return false;
// }

// template<
//     typename I,
//     typename F,
//     HALCHECK_REQUIRE(lib::is_iterator<I>()),
//     HALCHECK_REQUIRE(lib::is_invocable_r<bool, F>())>
// bool any_sorted(I begin, I end, F func) {
//   using R = typename std::iterator_traits<I>::reference;

//   if (begin == end)
//     return func();

//   for (auto i = begin; i != end; ++i) {
//     if (std::any_of(begin, end, [&](R other) { return other < *i; }))
//       continue;

//     std::swap(*begin, *i);

//     if (any_sorted(begin + 1, end, func))
//       return true;

//     std::swap(*begin, *i);
//   }

//   return false;
// }

}} // namespace halcheck::lib

#endif
