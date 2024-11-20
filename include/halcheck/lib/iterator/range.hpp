#ifndef HALCHECK_LIB_ITERATOR_RANGES_HPP
#define HALCHECK_LIB_ITERATOR_RANGES_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/pp.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>

namespace halcheck { namespace lib {

/**
 * @brief An implementation of std::ranges::enable_borrowed_range.
 * @see std::ranges::enable_borrowed_range
 * @ingroup lib-iterator
 */
template<class R>
struct enable_borrowed_range : std::false_type {};

inline namespace begin_cpo {

/**
 * @brief Gets an iterator to the first element of a range.
 * @par Signature
 * @code
 *   template<typename T>
 *   lib::iterator_t<T> begin(T &&value);
 * @endcode
 * @tparam T The type of range to query.
 * @param value The range to query.
 * @return An iterator to the first element of @p range.
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
private:
  template<typename T>
  using member = lib::iterator<decltype(std::declval<T &>().begin())>;

  template<typename T>
  using free = lib::iterator<decltype(begin(std::declval<T &>()))>;

public:
  template<typename T, std::size_t N>
  constexpr T *operator()(T (&value)[N]) const { // NOLINT
    return value + 0;
  };

  template<typename T, std::size_t N>
  constexpr const T *operator()(const T (&value)[N]) const { // NOLINT
    return value + 0;
  };

  template<typename T, std::size_t N>
  constexpr T *operator()(T *value) const { // NOLINT
    return value;
  };

  template<typename T, std::size_t N>
  constexpr const T *operator()(const T *value) const { // NOLINT
    return value;
  };

  template<
      typename T,
      HALCHECK_REQUIRE(std::is_lvalue_reference<T>() || lib::enable_borrowed_range<T>()),
      HALCHECK_REQUIRE(lib::is_detected<member, T>())>
  constexpr auto operator()(T &&value) const -> decltype(value.begin()) {
    return value.begin();
  }

  template<
      typename T,
      HALCHECK_REQUIRE(std::is_lvalue_reference<T>() || lib::enable_borrowed_range<T>()),
      HALCHECK_REQUIRE(!lib::is_detected<member, T>()),
      HALCHECK_REQUIRE(lib::is_detected<free, T>())>
  constexpr auto operator()(T &&value) const -> decltype(begin(value)) {
    return begin(value);
  }
} begin;

} // namespace begin_cpo

/**
 * @brief Obtains the iterator type of a range type.
 * @tparam T The type of range to query.
 * @ingroup lib-iterator
 */
template<typename T>
using iterator_t = decltype(lib::begin(std::declval<T &>()));

inline namespace end_cpo {

/**
 * @brief Gets an iterator to past the end of a range.
 * @par Signature
 * @code
 *   template<typename T>
 *   lib::iterator_t<T> end(T &&value);
 * @endcode
 * @tparam T The type of range to query.
 * @param value The range to query.
 * @return An iterator to past the end of @p range.
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
private:
  template<typename T>
  using member = lib::iterator<decltype(std::declval<T &&>().end())>;

  template<typename T>
  using free = lib::iterator<decltype(end(std::declval<T &&>()))>;

public:
  template<typename T, std::size_t N>
  constexpr T *operator()(T (&value)[N]) const { // NOLINT
    return value + N;
  };

  template<typename T, std::size_t N>
  constexpr const T *operator()(const T (&value)[N]) const { // NOLINT
    return value + N;
  };

  template<
      typename T,
      HALCHECK_REQUIRE(std::is_lvalue_reference<T>() || lib::enable_borrowed_range<T>()),
      HALCHECK_REQUIRE(lib::is_detected<member, T>())>
  constexpr auto operator()(T &&value) const -> decltype(value.end()) {
    return value.end();
  }

  template<
      typename T,
      HALCHECK_REQUIRE(std::is_lvalue_reference<T>() || lib::enable_borrowed_range<T>()),
      HALCHECK_REQUIRE(!lib::is_detected<member, T>()),
      HALCHECK_REQUIRE(lib::is_detected<free, T>())>
  constexpr auto operator()(T &&value) const -> decltype(end(value)) {
    return end(value);
  }
} end;

} // namespace end_cpo

/** @private */
template<typename T>
using range = lib::same<lib::iterator_t<T>, decltype(lib::end(std::declval<T &>()))>;

/**
 * @brief Determines whether the given type is a range.
 * @tparam T The type to query.
 * @see std::ranges::range
 * @ingroup lib-iterator
 */
template<typename T>
struct is_range : lib::is_detected<lib::range, T> {};

struct view_base {};

template<typename T>
struct enable_view : std::integral_constant<bool, std::is_base_of<lib::view_base, T>{}> {};

template<typename T>
using view = lib::void_t<lib::range<T>, lib::movable<T>, lib::enable_if_t<lib::enable_view<T>{}>>;

template<typename T>
struct is_view : lib::is_detected<lib::view, T> {};

// See https://en.cppreference.com/w/cpp/ranges/input_range

/** @private */
template<typename T>
using input_range = lib::void_t<lib::range<T>, lib::input_iterator<lib::iterator_t<T>>>;

/**
 * @brief Determines whether a type is a range whose iterators satisfy lib::is_input_iterator.
 * @tparam T The type to query.
 * @ingroup lib-iterator
 */
template<typename T>
struct is_input_range : lib::is_detected<lib::input_range, T> {};

/** @private */
template<typename T>
using forward_range = lib::void_t<lib::range<T>, lib::forward_iterator<lib::iterator_t<T>>>;

/**
 * @brief Determines whether a type is a range whose iterators satisfy lib::is_forward_iterator.
 * @tparam T The type to query.
 * @ingroup lib-iterator
 */
template<typename T>
struct is_forward_range : lib::is_detected<lib::forward_range, T> {};

/** @private */
template<typename T>
using bidirectional_range = lib::void_t<lib::range<T>, lib::bidirectional_iterator<lib::iterator_t<T>>>;

/**
 * @brief Determines whether a type is a range whose iterators satisfy lib::is_bidirectional_iterator.
 * @tparam T The type to query.
 * @ingroup lib-iterator
 */
template<typename T>
struct is_bidirectional_range : lib::is_detected<lib::bidirectional_range, T> {};

/** @private */
template<typename T>
using random_access_range = lib::void_t<lib::range<T>, lib::random_access_iterator<lib::iterator_t<T>>>;

/**
 * @brief Determines whether a type is a range whose iterators satisfy lib::is_random_access_iterator.
 * @tparam T The type to query.
 * @ingroup lib-iterator
 */
template<typename T>
struct is_random_access_range : lib::is_detected<lib::random_access_range, T> {};

/**
 * @brief Indicates whether lib::size should be disabled for a type.
 * @tparam T The type to query.
 * @details This type may be specialized to inherit from std::true_type in order to disable lib::size for a particular
 * type.
 * @ingroup lib-iterator
 */
template<typename T>
struct disable_sized_range : std::false_type {};

/**
 * @brief Obtains the size of a range.
 * @par Signature
 * @code
 *   template<typename R>
 *   lib::range_size_t<R> lib::size(R &&range);
 * @endcode
 * @tparam R The type of range to query.
 * @param range The range to query.
 * @return The number of elements in @p range.
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
private:
  template<typename T>
  using member = lib::enable_if_t<std::is_integral<decltype(std::declval<T>().size())>{}>;

  template<typename T>
  using free = lib::enable_if_t<std::is_integral<decltype(size(std::declval<T>()))>{}>;

public:
  template<typename T>
  constexpr void operator()(T *) const = delete; // NOLINT

  template<typename T, std::size_t N>
  constexpr std::size_t operator()(T (&)[N]) const noexcept { // NOLINT
    return N;
  }

  template<typename T, std::size_t N>
  constexpr std::size_t operator()(const T (&)[N]) const noexcept { // NOLINT
    return N;
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::disable_sized_range<lib::remove_cvref_t<T>>()),
      HALCHECK_REQUIRE(lib::is_detected<member, T>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(value.size())) -> decltype(value.size()) {
    return value.size();
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_detected<member, T>()),
      HALCHECK_REQUIRE(std::is_class<lib::remove_cvref_t<T>>() || std::is_enum<lib::remove_cvref_t<T>>()),
      HALCHECK_REQUIRE(!lib::disable_sized_range<lib::remove_cvref_t<T>>()),
      HALCHECK_REQUIRE(lib::is_detected<free, T>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(size(value))) -> decltype(size(value)) {
    return size(value);
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_detected<member, T>()),
      HALCHECK_REQUIRE(!lib::is_detected<free, T>()),
      HALCHECK_REQUIRE(lib::is_random_access_range<T>()),
      HALCHECK_REQUIRE(!lib::disable_sized_range<lib::remove_cvref_t<T>>()),
      HALCHECK_REQUIRE(lib::is_detected<
                       lib::make_unsigned_t,
                       decltype(lib::end(std::declval<T &>()) - lib::begin(std::declval<T &>()))>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(lib::end(value) - lib::begin(value)))
      -> decltype(lib::end(value) - lib::begin(value)) {
    return lib::end(value) - lib::begin(value);
  }
} size;

/** @private */
template<typename R>
using sized_range = lib::void_t<lib::range<R>, decltype(lib::size(std::declval<R &>()))>;

/**
 * @brief Determines whether a range type supports the lib::size operation.
 * @tparam R The type to query.
 * @ingroup lib-iterator
 */
template<typename R>
struct is_sized_range : lib::is_detected<lib::sized_range, R> {};

/**
 * @brief The type of value returned by lib::size.
 * @tparam R The type to query.
 * @ingroup lib-iterator
 */
template<typename R, HALCHECK_REQUIRE(lib::is_sized_range<R>())>
using range_size_t = decltype(lib::size(std::declval<R &>()));

/**
 * @brief The type of value returned by `operator-` for a range type's iterators.
 * @tparam R The type to query.
 * @ingroup lib-iterator
 */
template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_difference_t = lib::iter_difference_t<lib::iterator_t<R>>;

/**
 * @brief The type of element contained in a range.
 * @tparam R The type to query.
 * @ingroup lib-iterator
 */
template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_value_t = lib::iter_value_t<lib::iterator_t<R>>;

/**
 * @brief The type returned by `operator*` for a range type's iterators.
 * @tparam R The type to query.
 * @ingroup lib-iterator
 */
template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_reference_t = lib::iter_reference_t<lib::iterator_t<R>>;

/**
 * @brief Determines if a range is empty.
 * @par Signature
 * @code
 *   template<typename R>
 *   bool lib::empty(R &&range);
 * @endcode
 * @tparam R The type of range to query.
 * @param range The range to query.
 * @return A `bool` indicating whether the range is empty.
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
private:
  template<typename T>
  using member = lib::enable_if_t<std::is_integral<decltype(std::declval<T>().empty())>{}>;

public:
  template<typename T, HALCHECK_REQUIRE(lib::is_detected<member, T>())>
  constexpr bool operator()(T &&value) const noexcept(noexcept(bool(value.empty()))) {
    return bool(value.empty());
  }

  template<typename T, HALCHECK_REQUIRE(!lib::is_detected<member, T>()), HALCHECK_REQUIRE(lib::is_sized_range<T>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(lib::size(value) == 0))
      -> decltype(lib::size(value) == 0) {
    return lib::size(value) == 0;
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_detected<member, T>()),
      HALCHECK_REQUIRE(!lib::is_sized_range<T>()),
      HALCHECK_REQUIRE(lib::is_forward_range<T>())>
  constexpr bool operator()(T &&value) const noexcept(noexcept(bool(lib::end(value) == lib::begin(value)))) {
    return bool(lib::end(value) == lib::begin(value));
  }
} empty;

/** @private */
template<typename R>
using insertable_range = lib::void_t<
    lib::range<R>,
    lib::same<
        decltype(std::declval<R &>().insert(
            std::declval<lib::iterator_t<const R &>>(), std::declval<lib::range_value_t<R>>())),
        lib::iterator_t<R>>>;

/**
 * @brief Determines whether a range is insertable.
 * @tparam R The type to query.
 * @details A range is insertable if the expression `x.insert(i, y)` is valid, where
 * - `x` is an l-value reference of type @p R,
 * - `i` is a value of type lib::iterator_t<const R &>,
 * - `y` is a value of type lib::range_value_t<R>, and
 * - `x.insert(i, y)` has type lib::iterator_t<R>.
 * @ingroup lib-iterator
 */
template<typename R>
struct is_insertable_range : lib::is_detected<lib::insertable_range, R> {};

}} // namespace halcheck::lib

#endif
