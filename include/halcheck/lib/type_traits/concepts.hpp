#ifndef HALCHECK_LIB_TYPE_TRAITS_CONCEPTS_HPP
#define HALCHECK_LIB_TYPE_TRAITS_CONCEPTS_HPP

// IWYU pragma: private, include <halcheck/lib/type_traits.hpp>

#include <halcheck/lib/type_traits/compat.hpp>

#include <cstddef>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>

#define HALCHECK_REQUIRE(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int> = 0
#define HALCHECK_REQUIRE_(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int>

namespace halcheck { namespace lib {

namespace detail {
template<typename, template<typename...> class Op, typename... Args>
struct is_detected_helper : std::false_type {};

template<template<typename...> class Op, typename... Args>
struct is_detected_helper<lib::to_void<Op<Args...>>, Op, Args...> : std::true_type {};
} // namespace detail

template<template<typename...> class Op, typename... Args>
struct is_detected : detail::is_detected_helper<void, Op, Args...> {};

template<typename From, typename To>
using convertible = lib::enable_if_t<std::is_convertible<From, To>{}>;

template<typename T, typename U>
using same = lib::enable_if_t<std::is_same<T, U>{}>;

template<typename T>
using tuple_like = decltype(std::tuple_size<T>::value);

template<typename T>
struct is_tuple_like : lib::is_detected<lib::tuple_like, T> {};

template<class T, typename... Args>
using brace_constructible = decltype(T{std::declval<Args>()...});

template<class T, typename... Args>
struct is_brace_constructible : lib::is_detected<lib::brace_constructible, T, Args...> {};

namespace detail {
using std::swap;

template<typename T>
using swappable = decltype(swap(std::declval<T &>(), std::declval<T &>()));
} // namespace detail

using detail::swappable;

template<typename T>
using nothrow_swappable = lib::enable_if_t<noexcept(swap(std::declval<T &>(), std::declval<T &>()))>;

template<typename T>
struct is_swappable : lib::is_detected<lib::swappable, T> {};

template<typename T>
struct is_nothrow_swappable : lib::is_detected<lib::nothrow_swappable, T> {};

template<typename T>
using destructible = lib::enable_if_t<std::is_destructible<T>{}>;

template<typename T>
using default_constructible = lib::enable_if_t<std::is_default_constructible<T>{}>;

template<typename T>
using move_constructible = lib::enable_if_t<std::is_move_constructible<T>{}>;

template<typename T>
using move_assignable = lib::enable_if_t<std::is_move_assignable<T>{}>;

template<typename T>
using movable = lib::to_void<lib::move_constructible<T>, lib::move_assignable<T>>;

template<typename T>
struct is_movable : lib::is_detected<lib::movable, T> {};

template<typename T>
using copy_constructible = lib::enable_if_t<std::is_copy_constructible<T>{}>;

template<typename T>
using copy_assignable = lib::enable_if_t<std::is_copy_assignable<T>{}>;

template<typename T>
using copyable = lib::to_void<lib::copy_constructible<T>, lib::copy_assignable<T>>;

template<typename T>
struct is_copyable : lib::is_detected<lib::copyable, T> {};

template<typename B>
using boolean_testable =
    lib::to_void<lib::convertible<B, bool>, lib::convertible<decltype(!std::forward<B>(std::declval<B &>())), bool>>;

template<typename B>
struct is_boolean_testable : lib::is_detected<lib::boolean_testable, B> {};

template<typename T>
using equality_comparable = lib::to_void<
    lib::boolean_testable<decltype(std::declval<const T &>() == std::declval<const T &>())>,
    lib::boolean_testable<decltype(std::declval<const T &>() != std::declval<const T &>())>>;

template<typename T>
struct is_equality_comparable : lib::is_detected<lib::equality_comparable, T> {};

template<typename T>
using hashable = lib::same<decltype(std::declval<std::hash<T>>()(std::declval<T &>())), std::size_t>;

template<typename T>
struct is_hashable : lib::is_detected<lib::hashable, T> {};

template<typename T, typename Stream>
using printable = lib::same<decltype(std::declval<Stream &>() << std::declval<const T &>()), Stream &>;

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_printable : lib::is_detected<lib::printable, T, std::basic_ostream<CharT, Traits>> {};

template<typename T, typename Stream>
using parsable = lib::same<decltype(std::declval<Stream &>() >> std::declval<T &>()), Stream &>;

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_parsable : lib::is_detected<lib::parsable, T, std::basic_istream<CharT, Traits>> {};

template<typename, template<typename...> class>
struct is_specialization_of : std::false_type {};

template<typename... Args, template<typename...> class F>
struct is_specialization_of<F<Args...>, F> : std::true_type {};

template<typename T, template<typename...> class F>
using specialization_of = lib::enable_if_t<lib::is_specialization_of<T, F>{}>;

}} // namespace halcheck::lib

#endif
