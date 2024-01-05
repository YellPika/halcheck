#ifndef HALCHECK_LIB_TYPE_TRAITS_HPP
#define HALCHECK_LIB_TYPE_TRAITS_HPP

#include <ostream>
#include <type_traits>
#include <utility>

#define HALCHECK_REQUIRE(...) ::halcheck::lib::enable_if_t<(__VA_ARGS__), int> = 0

namespace halcheck { namespace lib {

#if __cplusplus >= 201703L
using std::conjunction;
#else
template<typename... Args>
struct conjunction;

template<>
struct conjunction<> : std::true_type {};

template<typename T>
struct conjunction<T> : T {};

template<typename T, typename... Args>
struct conjunction<T, Args...> : std::conditional<bool(T::value), conjunction<Args...>, T>::type {};
#endif

#if __cplusplus >= 201402L
using std::common_type_t;
using std::decay_t;
using std::enable_if_t;
using std::make_unsigned_t;
using std::remove_cv_t;
#else
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
#endif

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
using is_streamable = decltype(std::declval<std::ostream &>() << std::declval<T>());
} // namespace detail

template<typename T>
struct is_streamable : lib::is_detected<detail::is_streamable, T> {};

namespace detail {
template<typename T>
using is_tuple_like_helper = decltype(std::tuple_size<T>::value);
} // namespace detail

template<typename T>
struct is_tuple_like : lib::is_detected<detail::is_tuple_like_helper, T> {};

namespace detail {
template<typename T>
struct dummy {
  template<typename U, HALCHECK_REQUIRE(std::is_same<T, U>())>
  operator U() {
    return value;
  }

  T value;
};
} // namespace detail

template<typename T>
struct is_streamable_enum : lib::conjunction<std::is_enum<T>, lib::is_streamable<detail::dummy<T>>> {};

namespace detail {
template<template<typename...> class F, typename... Args>
void apply_helper(...);

template<template<typename...> class F, typename... Args>
F<Args...> apply_helper(int);
} // namespace detail

template<template<typename...> class F, typename... Args>
using apply = decltype(detail::apply_helper<F, Args...>(0));

template<typename T>
using type_identity_t = T;

}} // namespace halcheck::lib

#endif
