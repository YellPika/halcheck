#ifndef HALCHECK_LIB_ITERATOR_RANGES_HPP
#define HALCHECK_LIB_ITERATOR_RANGES_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/borrowed_range

template<class R>
struct enable_borrowed_range : std::false_type {};

// See https://en.cppreference.com/w/cpp/ranges/begin

static const struct {
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
  constexpr T *operator()(T (&value)[]) const { // NOLINT
    return value + 0;
  };

  template<typename T, std::size_t N>
  constexpr const T *operator()(const T (&value)[]) const { // NOLINT
    return value + 0;
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

// See https://en.cppreference.com/w/cpp/ranges/iterator_t

template<typename T>
using iterator_t = decltype(lib::begin(std::declval<T &>()));

// See https://en.cppreference.com/w/cpp/ranges/end

static const class {
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

// See https://en.cppreference.com/w/cpp/ranges/range

template<typename T>
using range = lib::same<lib::iterator_t<T>, decltype(lib::end(std::declval<T &>()))>;

template<typename T>
struct is_range : lib::is_detected<lib::range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/view

struct view_base {};

template<typename T>
struct enable_view : std::integral_constant<bool, std::is_base_of<lib::view_base, T>{}> {};

template<typename T>
using view = lib::to_void<lib::range<T>, lib::movable<T>, lib::enable_if_t<lib::enable_view<T>{}>>;

template<typename T>
struct is_view : lib::is_detected<lib::view, T> {};

// See https://en.cppreference.com/w/cpp/ranges/input_range

template<typename T>
using input_range = lib::to_void<lib::range<T>, lib::input_iterator<lib::iterator_t<T>>>;

template<typename T>
struct is_input_range : lib::is_detected<lib::input_range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/forward_range

template<typename T>
using forward_range = lib::to_void<lib::input_range<T>, lib::forward_iterator<lib::iterator_t<T>>>;

template<typename T>
struct is_forward_range : lib::is_detected<lib::forward_range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/bidirectional_range

template<typename T>
using bidirectional_range = lib::to_void<lib::input_range<T>, lib::bidirectional_iterator<lib::iterator_t<T>>>;

template<typename T>
struct is_bidirectional_range : lib::is_detected<lib::bidirectional_range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/random_access_range

template<typename T>
using random_access_range = lib::to_void<lib::input_range<T>, lib::random_access_iterator<lib::iterator_t<T>>>;

template<typename T>
struct is_random_access_range : lib::is_detected<lib::random_access_range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/sized_range

template<typename>
struct disable_sized_range : std::false_type {};

// See https://en.cppreference.com/w/cpp/ranges/size

static const struct {
private:
  template<typename T>
  using member = lib::enable_if_t<std::is_integral<decltype(std::declval<T>().size())>{}>;

  template<typename T>
  using free = lib::enable_if_t<std::is_integral<decltype(size(std::declval<T>()))>{}>;

public:
  template<typename T>
  constexpr void operator()(T (&value)[]) const = delete; // NOLINT

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
  constexpr auto operator()(T &&value) const
      noexcept(noexcept(lib::end(value) - lib::begin(value))) -> decltype(lib::end(value) - lib::begin(value)) {
    return lib::end(value) - lib::begin(value);
  }
} size;

// See https://en.cppreference.com/w/cpp/ranges/sized_range

template<typename T>
using sized_range = lib::to_void<lib::range<T>, decltype(lib::size(std::declval<T &>()))>;

template<typename T>
struct is_sized_range : lib::is_detected<lib::sized_range, T> {};

// See https://en.cppreference.com/w/cpp/ranges/range_size_t

template<typename R, HALCHECK_REQUIRE(lib::is_sized_range<R>())>
using range_size_t = decltype(lib::size(std::declval<R &>()));

template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_difference_t = lib::iter_difference_t<lib::iterator_t<R>>;

template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_value_t = lib::iter_value_t<lib::iterator_t<R>>;

// See https://en.cppreference.com/w/cpp/ranges/range_reference_t

template<typename R, HALCHECK_REQUIRE(lib::is_range<R>())>
using range_reference_t = lib::iter_reference_t<lib::iterator_t<R>>;

// See https://en.cppreference.com/w/cpp/ranges/empty

static const struct {
private:
  template<typename T>
  using member = lib::enable_if_t<std::is_integral<decltype(std::declval<T>().empty())>{}>;

public:
  template<typename T, HALCHECK_REQUIRE(lib::is_detected<member, T>())>
  constexpr bool operator()(T &&value) const noexcept(noexcept(bool(value.empty()))) {
    return bool(value.empty());
  }

  template<typename T, HALCHECK_REQUIRE(!lib::is_detected<member, T>()), HALCHECK_REQUIRE(lib::is_sized_range<T>())>
  constexpr auto operator()(T &&value) const
      noexcept(noexcept(lib::size(value) == 0)) -> decltype(lib::size(value) == 0) {
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

template<typename T>
using insertable_range = lib::to_void<
    lib::range<T>,
    lib::same<
        decltype(std::declval<T &>().insert(
            std::declval<lib::iterator_t<const T &>>(), std::declval<lib::range_value_t<T>>())),
        lib::iterator_t<T>>>;

template<typename T>
struct is_insertable_range : lib::is_detected<lib::insertable_range, T> {};

}} // namespace halcheck::lib

#endif
