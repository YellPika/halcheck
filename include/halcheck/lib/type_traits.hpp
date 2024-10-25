#ifndef HALCHECK_LIB_TYPE_TRAITS_HPP
#define HALCHECK_LIB_TYPE_TRAITS_HPP

/// @file
/// @brief Metaprogramming utilities
/// @see https://en.cppreference.com/w/cpp/header/type_traits

#include <cstddef>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits> // IWYU pragma: export

#ifdef HALCHECK_DOXYGEN
/// @brief Expands to a template argument that is only valid if the given argument evaluates to `true`.
#define HALCHECK_REQUIRE(...)
/// @brief A version of @ref HALCHECK_REQUIRE usable in forward and friend declarations.
#define HALCHECK_REQUIRE_(...)
#else
#define HALCHECK_REQUIRE(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int> = 0
#define HALCHECK_REQUIRE_(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int>
#endif

namespace halcheck { namespace lib {

/// @brief An implementation of std::void_t.
/// @see std::void_t
template<typename...>
using void_t = void;

/// @brief An implementation of std::conditional_t.
/// @see std::conditional_t
template<bool Cond, typename T, typename F>
using conditional_t = typename std::conditional<Cond, T, F>::type;

/// @brief An implementation of std::conjunction.
/// @see std::conjunction
template<typename... Args>
struct conjunction;

template<>
struct conjunction<> : std::true_type {};

template<typename T>
struct conjunction<T> : T {};

template<typename T, typename... Args>
struct conjunction<T, Args...> : lib::conditional_t<bool(T::value), conjunction<Args...>, T> {};

/// @brief An implementation of std::disjunction.
/// @see std::disjunction
template<typename... Args>
struct disjunction;

template<>
struct disjunction<> : std::false_type {};

template<typename T>
struct disjunction<T> : T {};

template<typename T, typename... Args>
struct disjunction<T, Args...> : lib::conditional_t<bool(T::value), T, disjunction<Args...>> {};

/// @brief An implementation of std::enable_if_t.
/// @see std::enable_if_t
template<bool Cond, typename T = void>
using enable_if_t = typename std::enable_if<Cond, T>::type;

/// @brief An implementation of std::decay_t.
/// @see std::decay_t
template<typename T>
using decay_t = typename std::decay<T>::type;

/// @brief An implementation of std::remove_cv_t.
/// @see std::remove_cv_t
template<typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

/// @brief An implementation of std::remove_extent_t.
/// @see std::remove_extent_t
template<typename T>
using remove_extent_t = typename std::remove_extent<T>::type;

/// @brief An implementation of std::make_unsigned_t.
/// @see std::make_unsigned_t
template<typename T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

/// @brief An implementation of std::common_type_t.
/// @see std::common_type_t
template<typename... Args>
using common_type_t = typename std::common_type<Args...>::type;

/// @brief An implementation of std::remove_cvref_t.
/// @see std::remove_cvref_t
template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

/// @brief An implementation of std::type_identity.
/// @see std::type_identity
template<typename T, typename...>
struct type_identity {
  using type = T;
};

/// @brief An implementation of std::type_identity_t.
/// @see std::type_identity_t
template<typename T, typename...>
using type_identity_t = T;

/// @brief Provides compile-time access the the types of the elements of a tuple.
/// @see std::tuple_element
template<std::size_t I, typename T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

/// @brief An implementation of std::remove_reference_t.
/// @see std::remove_reference_t
template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

/// @brief An implementation of std::remove_const_t.
/// @see std::remove_const_t
template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

/// @brief An implementation of std::make_signed_t.
/// @see std::make_signed_t
template<typename T>
using make_signed_t = typename std::make_signed<T>::type;

/// @brief An implementation of std::add_lvalue_reference_t.
/// @see std::add_lvalue_reference_t
template<typename T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

/// @brief An implementation of std::add_rvalue_reference_t.
/// @see std::add_rvalue_reference_t
template<typename T>
using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

/// @brief An implementation of std::add_const_t.
/// @see std::add_const_t
template<typename T>
using add_const_t = typename std::add_const<T>::type;

/// @brief An implementation of std::add_pointer_t.
/// @see std::add_pointer_t
template<typename T>
using add_pointer_t = typename std::add_pointer<T>::type;

/// @brief An implementation of std::remove_pointer_t.
/// @see std::remove_pointer_t
template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

namespace detail {
template<typename, template<typename...> class Op, typename... Args>
struct is_detected_helper : std::false_type {};

template<template<typename...> class Op, typename... Args>
struct is_detected_helper<lib::void_t<Op<Args...>>, Op, Args...> : std::true_type {};
} // namespace detail

/// @brief An implementation of std::experimental::is_detected.
/// @see std::experimental::is_detected
template<template<typename...> class Op, typename... Args>
struct is_detected : detail::is_detected_helper<void, Op, Args...> {};

/// @private
template<typename From, typename To>
using convertible = lib::enable_if_t<std::is_convertible<From, To>{}>;

/// @private
template<typename T, typename U>
using same = lib::enable_if_t<std::is_same<T, U>{}>;

/// @private
template<typename T, typename U>
using base_of = lib::enable_if_t<std::is_base_of<T, U>{}>;

/// @private
template<typename T>
using tuple_like = decltype(std::tuple_size<T>::value);

/// @private
template<typename T>
struct is_tuple_like : lib::is_detected<lib::tuple_like, T> {};

/// @private
template<class T, typename... Args>
using brace_constructible = decltype(T{std::declval<Args>()...});

/// @brief Determines if a type is constructible from a given set of argument types using initiailizer-list-style
/// construction.
/// @tparam T The type of value to construct.
/// @tparam Args The types of arguments to construct a @p T with.
template<class T, typename... Args>
struct is_brace_constructible : lib::is_detected<lib::brace_constructible, T, Args...> {};

namespace detail {
using std::swap;

/// @private
template<typename T>
using swappable = decltype(swap(std::declval<T &>(), std::declval<T &>()));

/// @private
template<typename T>
using nothrow_swappable = lib::enable_if_t<noexcept(swap(std::declval<T &>(), std::declval<T &>()))>;
} // namespace detail

using detail::nothrow_swappable;
using detail::swappable;

/// @brief An implementation of std::is_swappable.
/// @see std::is_swappable
template<typename T>
struct is_swappable : lib::is_detected<lib::swappable, T> {};

/// @brief An implementation of std::is_nothrow_swappable.
/// @see std::is_nothrow_swappable
template<typename T>
struct is_nothrow_swappable : lib::is_detected<lib::nothrow_swappable, T> {};

/// @private
template<typename T>
using destructible = lib::enable_if_t<std::is_destructible<T>{}>;

/// @private
template<typename T>
using default_constructible = lib::enable_if_t<std::is_default_constructible<T>{}>;

/// @private
template<typename T>
using move_constructible = lib::enable_if_t<std::is_move_constructible<T>{}>;

/// @private
template<typename T>
using move_assignable = lib::enable_if_t<std::is_move_assignable<T>{}>;

/// @private
template<typename T>
using movable = lib::void_t<lib::move_constructible<T>, lib::move_assignable<T>>;

/// @brief Determines if a type is move constructible and move assignable.
/// @tparam T The type to query.
template<typename T>
struct is_movable : lib::is_detected<lib::movable, T> {};

/// @private
template<typename T>
using copy_constructible = lib::enable_if_t<std::is_copy_constructible<T>{}>;

/// @private
template<typename T>
using copy_assignable = lib::enable_if_t<std::is_copy_assignable<T>{}>;

/// @private
template<typename T>
using copyable = lib::void_t<lib::copy_constructible<T>, lib::copy_assignable<T>>;

/// @brief Determines if a type is copy constructible and copy assignable.
/// @tparam T The type to query.
template<typename T>
struct is_copyable : lib::is_detected<lib::copyable, T> {};

template<typename B>
using boolean_testable =
    lib::void_t<lib::convertible<B, bool>, lib::convertible<decltype(!std::forward<B>(std::declval<B &>())), bool>>;

/// @brief Determines if a type is satisfies the
/// [boolean-testable](https://en.cppreference.com/w/cpp/concepts/boolean-testable) concept.
/// @tparam T The type to query.
/// @see https://en.cppreference.com/w/cpp/concepts/boolean-testable
template<typename B>
struct is_boolean_testable : lib::is_detected<lib::boolean_testable, B> {};

/// @private
template<typename T>
using equality_comparable = lib::void_t<
    lib::boolean_testable<decltype(std::declval<const T &>() == std::declval<const T &>())>,
    lib::boolean_testable<decltype(std::declval<const T &>() != std::declval<const T &>())>>;

/// @brief Determines if a type satisfies the
/// [EqualityComparable](https://en.cppreference.com/w/cpp/named_req/EqualityComparable) concept.
/// @tparam T The type to query.
/// @see https://en.cppreference.com/w/cpp/named_req/EqualityComparable
template<typename T>
struct is_equality_comparable : lib::is_detected<lib::equality_comparable, T> {};

/// @private
template<typename T>
using hashable = lib::same<decltype(std::declval<std::hash<T>>()(std::declval<T &>())), std::size_t>;

/// @brief Determines if a type has a valid std::hash specialization.
/// @tparam T The type to query.
template<typename T>
struct is_hashable : lib::is_detected<lib::hashable, T> {};

/// @private
template<typename T, typename Stream>
using printable = lib::same<decltype(std::declval<Stream &>() << std::declval<const T &>()), Stream &>;

/// @brief Determines whether a type is printable (i.e. can be output on a <tt> std::basic_ostream </tt>).
///
/// @tparam T The type to check.
/// @tparam CharT
/// @tparam Traits
/// @ingroup meta
template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_printable : lib::is_detected<lib::printable, T, std::basic_ostream<CharT, Traits>> {};

/// @private
template<typename T, typename Stream>
using parsable = lib::same<decltype(std::declval<Stream &>() >> std::declval<T &>()), Stream &>;

/// @brief Determines whether a type is parsable (i.e. can be read from a <tt> std::basic_istream </tt>).
///
/// @tparam T The type to check.
/// @tparam CharT
/// @tparam Traits
/// @ingroup meta
template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_parsable : lib::is_detected<lib::parsable, T, std::basic_istream<CharT, Traits>> {};

/// @brief Determines if a type is a specialization of a template class.
/// @tparam T The type to query.
/// @tparam F The template class to determine if @p T is a specialization of.
template<typename T, template<typename...> class F>
struct is_specialization_of : std::false_type {};

template<typename... Args, template<typename...> class F>
struct is_specialization_of<F<Args...>, F> : std::true_type {};

/// @private
template<typename T, template<typename...> class F>
using specialization_of = lib::enable_if_t<lib::is_specialization_of<T, F>{}>;

}} // namespace halcheck::lib

#endif
