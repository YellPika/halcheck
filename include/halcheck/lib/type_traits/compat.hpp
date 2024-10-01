#ifndef HALCHECK_LIB_TYPE_TRAITS_TYPEDEFS_HPP
#define HALCHECK_LIB_TYPE_TRAITS_TYPEDEFS_HPP

// IWYU pragma: private, include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits> // IWYU pragma: export

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
using remove_extent_t = typename std::remove_extent<T>::type;

template<typename T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

template<typename... Args>
using common_type_t = typename std::common_type<Args...>::type;

template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename T, typename...>
struct type_identity {
  using type = T;
};

template<typename T, typename...>
using type_identity_t = T;

template<std::size_t I, typename T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

template<typename T>
using make_signed_t = typename std::make_signed<T>::type;

template<typename T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

template<typename T>
using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

template<typename T>
using add_pointer_t = typename std::add_pointer<T>::type;

template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

}} // namespace halcheck::lib

#endif
