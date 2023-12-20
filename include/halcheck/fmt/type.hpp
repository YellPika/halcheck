#ifndef HALCHECK_FMT_TYPE_HPP
#define HALCHECK_FMT_TYPE_HPP

#include <halcheck/fmt/indent.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/typeinfo.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <initializer_list>
#include <map>
#include <ostream>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace halcheck { namespace fmt {

/// @brief A proxy type used for printing the names of class templates. For
///        custom printing, provide an overload of operator<<(std::ostream &,
///        fmt::type_fun<T>) for your type. Printed values should ideally be
///        compilable code that evaluates to the original type.
/// @tparam F The class template to print.
template<template<typename...> class F>
struct type_fun {};

inline std::ostream &operator<<(std::ostream &os, type_fun<std::set>) { return os << "std::set"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::map>) { return os << "std::map"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::pair>) { return os << "std::pair"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::queue>) { return os << "std::queue"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::stack>) { return os << "std::stack"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::tuple>) { return os << "std::tuple"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::vector>) { return os << "std::vector"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::allocator>) { return os << "std::allocator"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::char_traits>) { return os << "std::char_traits"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::basic_string>) { return os << "std::basic_string"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<std::initializer_list>) {
  return os << "std::initializer_list";
}

inline std::ostream &operator<<(std::ostream &os, type_fun<lib::variant>) { return os << "halcheck::lib::variant"; }
inline std::ostream &operator<<(std::ostream &os, type_fun<lib::optional>) { return os << "halcheck::lib::optional"; }

template<template<typename...> class F>
inline std::ostream &operator<<(std::ostream &os, type_fun<F>) {
  return os << "[unknown template]";
}

/// @brief A proxy type used for printing types. For custom printing, provide an
///        overload of operator<<(std::ostream &, fmt::type<T>) for your type.
///        Printed values should ideally be compilable code that evaluates to
///        the original type.
/// @tparam T The type to print.
template<typename T>
struct type {};

inline std::ostream &operator<<(std::ostream &os, type<char>) { return os << "char"; }
inline std::ostream &operator<<(std::ostream &os, type<signed char>) { return os << "signed char"; }
inline std::ostream &operator<<(std::ostream &os, type<unsigned char>) { return os << "unsigned char"; }
inline std::ostream &operator<<(std::ostream &os, type<int>) { return os << "int"; }
inline std::ostream &operator<<(std::ostream &os, type<unsigned int>) { return os << "unsigned int"; }
inline std::ostream &operator<<(std::ostream &os, type<short>) { return os << "short"; }
inline std::ostream &operator<<(std::ostream &os, type<unsigned short>) { return os << "unsigned short"; }
inline std::ostream &operator<<(std::ostream &os, type<long>) { return os << "long"; }
inline std::ostream &operator<<(std::ostream &os, type<unsigned long>) { return os << "unsigned long"; }
inline std::ostream &operator<<(std::ostream &os, type<long long>) { return os << "long long"; }
inline std::ostream &operator<<(std::ostream &os, type<unsigned long long>) { return os << "unsigned long long"; }
inline std::ostream &operator<<(std::ostream &os, type<float>) { return os << "float"; }
inline std::ostream &operator<<(std::ostream &os, type<double>) { return os << "double"; }
inline std::ostream &operator<<(std::ostream &os, type<long double>) { return os << "long double"; }
inline std::ostream &operator<<(std::ostream &os, type<std::string>) { return os << "std::string"; }

template<typename T>
std::ostream &operator<<(std::ostream &os, type<const T>) {
  return os << "const " << type<T>();
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<volatile T>) {
  return os << "volatile " << type<T>();
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<const volatile T>) {
  return os << "const volatile " << type<T>();
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<T &>) {
  return os << type<T>() << " &";
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<T &&>) {
  return os << type<T>() << " &&";
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<T *>) {
  return os << type<T>() << " *";
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<T **>) {
  return os << type<T *>() << "*";
}

namespace detail {
template<typename... Ts>
struct args {};

template<
    template<typename...>
    class F,
    typename... Init,
    typename... Tail,
    HALCHECK_REQUIRE(std::is_same<lib::apply<F, Init...>, lib::apply<F, Init..., Tail...>>())>
void print_args(std::ostream &os, args<Init...>, args<Tail...>) {
  switch (sizeof...(Init)) {
  case 0:
    os << fmt::type_fun<F>() << "<>";
    break;
  case 1:
    os << fmt::type_fun<F>() << "<";
    lib::ignore = {(os << type<Init>(), 0)...};
    os << ">";
    break;
  default:
    os << fmt::type_fun<F>() << "<\n";
    std::size_t i = 0;
    lib::ignore = {(os << (i++ > 0 ? ", \n" : "") << type<Init>(), 0)...};
    os << ">";
    break;
  }
}

template<
    template<typename...>
    class F,
    typename... Init,
    typename T,
    typename... Tail,
    HALCHECK_REQUIRE(!std::is_same<lib::apply<F, Init...>, lib::apply<F, Init..., T, Tail...>>())>
void print_args(std::ostream &os, args<Init...>, args<T, Tail...>) {
  print_args<F>(os, args<Init..., T>{}, args<Tail...>{});
}
} // namespace detail

template<typename... Ts>
std::ostream &operator<<(std::ostream &os, type<lib::variant<Ts...>>) {
  fmt::indent indent(os);
  detail::print_args<lib::variant>(os, detail::args<Ts...>{}, detail::args<>{});
  return os;
}

template<template<typename...> class F, typename... Ts>
std::ostream &operator<<(std::ostream &os, type<F<Ts...>>) {
  fmt::indent indent(os);
  detail::print_args<F>(os, detail::args<Ts...>{}, detail::args<>{});
  return os;
}

template<typename T>
std::ostream &operator<<(std::ostream &os, type<T>) {
#ifdef HALCHECK_RTTI
  return os << lib::nameof<T>();
#else
  return os << "[unknown]";
#endif
}
}} // namespace halcheck::fmt

#endif
