#ifndef HALCHECK_LIB_TYPE_TRAITS_HPP
#define HALCHECK_LIB_TYPE_TRAITS_HPP

#include <cstddef>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits> // IWYU pragma: export
#include <utility>

#define HALCHECK_REQUIRE(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int> = 0
#define HALCHECK_REQUIRE_(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int>

namespace halcheck { namespace lib {

template<typename...>
using to_void = void;

template<bool Cond, typename T, typename F>
using conditional_t = typename std::conditional<Cond, T, F>::type;

template<typename... Args>
struct conjunction;

template<>
struct conjunction<> : std::true_type {};

template<typename T>
struct conjunction<T> : T {};

template<typename T, typename... Args>
struct conjunction<T, Args...> : lib::conditional_t<bool(T::value), conjunction<Args...>, T> {};

template<typename... Args>
struct disjunction;

template<>
struct disjunction<> : std::false_type {};

template<typename T>
struct disjunction<T> : T {};

template<typename T, typename... Args>
struct disjunction<T, Args...> : lib::conditional_t<bool(T::value), T, disjunction<Args...>> {};

template<bool Cond, typename T = void>
using enable_if_t = typename std::enable_if<Cond, T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template<typename T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

template<typename... Args>
using common_type_t = typename std::common_type<Args...>::type;

template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

namespace detail {
template<typename, template<typename...> class Op, typename... Args>
struct is_detected_helper : std::false_type {};

template<template<typename...> class Op, typename... Args>
struct is_detected_helper<decltype(void(std::declval<Op<Args...>>)), Op, Args...> : std::true_type {};
} // namespace detail

template<template<typename...> class Op, typename... Args>
struct is_detected : detail::is_detected_helper<void, Op, Args...> {};

namespace detail {
template<typename T>
using is_tuple_like_helper = decltype(std::tuple_size<T>::value);
} // namespace detail

template<typename T>
struct is_tuple_like : lib::is_detected<detail::is_tuple_like_helper, T> {};

template<std::size_t I, typename T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

template<typename T>
using type_identity_t = T;

namespace detail {
template<class T, typename... Args>
decltype(void(T{std::declval<Args>()...}), std::true_type()) is_brace_constructible_helper(int);

template<class T, typename... Args>
std::false_type is_brace_constructible_helper(...);
} // namespace detail

template<class T, typename... Args>
struct is_brace_constructible : decltype(detail::is_brace_constructible_helper<T, Args...>(0)) {};

template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

namespace detail {
template<typename T, typename U>
using is_equality_comparable_helper = lib::enable_if_t<
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() == std::declval<const lib::remove_reference_t<U> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<U> &>() == std::declval<const lib::remove_reference_t<T> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() != std::declval<const lib::remove_reference_t<U> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<U> &>() != std::declval<const lib::remove_reference_t<T> &>()),
        bool>()>;
} // namespace detail

template<typename T, typename U = T>
struct is_equality_comparable : lib::is_detected<detail::is_equality_comparable_helper, T, U> {};

namespace detail {
using std::swap;

template<typename T>
using is_swappable_helper = decltype(swap(std::declval<T &>(), std::declval<T &>()));

template<typename T>
using is_nothrow_swappable_helper = lib::enable_if_t<noexcept(swap(std::declval<T &>(), std::declval<T &>()))>;
} // namespace detail

template<typename T>
struct is_swappable : lib::is_detected<detail::is_swappable_helper, T> {};

template<typename T>
struct is_nothrow_swappable : lib::is_detected<detail::is_nothrow_swappable_helper, T> {};

template<typename T>
using is_movable = lib::
    conjunction<std::is_object<T>, std::is_move_constructible<T>, std::is_assignable<T &, T>, lib::is_swappable<T>>;

template<typename T>
using is_copyable = lib::conjunction<
    std::is_copy_constructible<T>,
    lib::is_movable<T>,
    std::is_assignable<T &, T &>,
    std::is_assignable<T &, const T &>,
    std::is_assignable<T &, const T>>;

namespace detail {
template<typename T>
using is_totally_ordered_helper = lib::enable_if_t<
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() < std::declval<const lib::remove_reference_t<T> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() <= std::declval<const lib::remove_reference_t<T> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() > std::declval<const lib::remove_reference_t<T> &>()),
        bool>() &&
    std::is_convertible<
        decltype(std::declval<const lib::remove_reference_t<T> &>() >= std::declval<const lib::remove_reference_t<T> &>()),
        bool>()>;
} // namespace detail

template<typename T>
using is_totally_ordered =
    lib::conjunction<lib::is_equality_comparable<T>, lib::is_detected<detail::is_totally_ordered_helper, T>>;

namespace detail {
template<typename T>
using is_hashable_helper =
    lib::enable_if_t<std::is_same<decltype(std::declval<std::hash<T>>()(std::declval<T>())), std::size_t>{}>;
} // namespace detail

template<typename T>
struct is_hashable : lib::is_detected<detail::is_hashable_helper, T> {};

namespace detail {
template<typename T, typename Stream>
using is_printable_helper =
    lib::enable_if_t<std::is_same<decltype(std::declval<Stream &>() << std::declval<const T &>()), Stream &>{}>;
} // namespace detail

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_printable : lib::is_detected<detail::is_printable_helper, T, std::basic_ostream<CharT, Traits>> {};

namespace detail {
template<typename T, typename Stream>
using is_parsable_helper =
    lib::enable_if_t<std::is_same<decltype(std::declval<Stream &>() >> std::declval<T &>()), Stream &>{}>;
} // namespace detail

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_parsable : lib::is_detected<detail::is_parsable_helper, T, std::basic_istream<CharT, Traits>> {};

template<typename, template<typename...> class>
struct is_specialization_of : std::false_type {};

template<typename... Args, template<typename...> class F>
struct is_specialization_of<F<Args...>, F> : std::true_type {};

}} // namespace halcheck::lib

#endif
