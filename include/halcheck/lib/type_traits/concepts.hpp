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

template<typename T, typename U, template<typename> class QT, template<typename> class QU>
struct basic_common_reference {};

template<typename T, typename U, template<typename> class QT, template<typename> class QU>
using basic_common_reference_t = typename basic_common_reference<T, U, QT, QU>::type;

namespace detail {

template<typename T>
struct qualifier {
  template<typename U>
  using type = U;
};

template<typename T>
struct qualifier<const T> {
  template<typename U>
  using type = const U;
};

template<typename T>
struct qualifier<volatile T> {
  template<typename U>
  using type = volatile U;
};

template<typename T>
struct qualifier<const volatile T> {
  template<typename U>
  using type = const volatile U;
};

} // namespace detail

template<typename T, typename U>
using basic_common_reference_of_t = lib::basic_common_reference_t<
    lib::remove_cvref_t<T>,
    lib::remove_cvref_t<U>,
    detail::qualifier<T>::template type,
    detail::qualifier<U>::template type>;

namespace detail {

template<typename T, typename U>
using simple_common_reference_helper =
    decltype(false ? std::declval<typename detail::qualifier<T>::template type<U> &>() : std::declval<typename detail::qualifier<U>::template type<T> &>());

template<typename T, typename U, typename = void>
struct simple_common_reference {};

template<typename T, typename U>
struct simple_common_reference<
    T &,
    U &,
    lib::enable_if_t<std::is_reference<detail::simple_common_reference_helper<T, U>>{}>> {
  using type = detail::simple_common_reference_helper<T, U>;
};

template<typename T, typename U>
struct simple_common_reference<
    T &&,
    U &&,
    lib::enable_if_t<
        std::is_reference<detail::simple_common_reference_helper<T, U>>() &&
        std::is_convertible<T &&, detail::simple_common_reference_helper<T, U>>()>> {
  using type = lib::remove_reference_t<detail::simple_common_reference_helper<T, U>> &&;
};

template<typename T, typename U>
struct simple_common_reference<
    T &,
    U &&,
    lib::enable_if_t<
        std::is_reference<detail::simple_common_reference_helper<T, const U>>() &&
        std::is_convertible<U &&, detail::simple_common_reference_helper<T, const U>>()>> {
  using type = detail::simple_common_reference_helper<T, const U>;
};

template<typename T, typename U>
struct simple_common_reference<
    T &&,
    U &,
    lib::enable_if_t<
        std::is_reference<detail::simple_common_reference_helper<const T, U>>() &&
        std::is_convertible<T &&, detail::simple_common_reference_helper<const T, U>>()>> {
  using type = detail::simple_common_reference_helper<const T, U>;
};

template<typename T, typename U>
using simple_common_reference_t = typename detail::simple_common_reference<T, U>::type;

template<typename, typename...>
struct common_reference_helper;

template<typename... Args>
struct common_reference : common_reference_helper<void, Args...> {};

template<typename... Args>
using common_reference_t = typename common_reference<Args...>::type;

template<typename T, typename U>
using common_reference_1 = detail::simple_common_reference_t<T, U>;

template<typename T, typename U>
using common_reference_2 = lib::basic_common_reference_of_t<T, U>;

template<typename T>
T val();

template<typename T, typename U>
using common_reference_3 = decltype(false ? val<T>() : val<U>());

template<typename T, typename U>
using common_reference_4 = lib::common_type_t<T, U>;

template<typename T>
struct common_reference_helper<void, T> {
  using type = T;
};

template<typename T, typename U>
struct common_reference_helper<lib::enable_if_t<lib::is_detected<detail::common_reference_1, T, U>{}>, T, U> {
  using type = detail::common_reference_1<T, U>;
};

template<typename T, typename U>
struct common_reference_helper<
    lib::enable_if_t<
        !lib::is_detected<detail::common_reference_1, T, U>{} && lib::is_detected<detail::common_reference_2, T, U>{}>,
    T,
    U> {
  using type = detail::common_reference_2<T, U>;
};

template<typename T, typename U>
struct common_reference_helper<
    lib::enable_if_t<
        !lib::is_detected<detail::common_reference_1, T, U>{} &&
        !lib::is_detected<detail::common_reference_2, T, U>{} && lib::is_detected<detail::common_reference_3, T, U>{}>,
    T,
    U> {
  using type = detail::common_reference_3<T, U>;
};

template<typename T, typename U>
struct common_reference_helper<
    lib::enable_if_t<
        !lib::is_detected<detail::common_reference_1, T, U>{} &&
        !lib::is_detected<detail::common_reference_2, T, U>{} &&
        !lib::is_detected<detail::common_reference_3, T, U>{} && lib::is_detected<detail::common_reference_4, T, U>{}>,
    T,
    U> {
  using type = detail::common_reference_4<T, U>;
};

template<typename T, typename U, typename V, typename... Args>
struct common_reference_helper<
    lib::to_void<detail::common_reference_t<detail::common_reference_t<T, U>, V, Args...>>,
    T,
    U,
    V,
    Args...> {
  using type = detail::common_reference_t<detail::common_reference_t<T, U>, V, Args...>;
};

} // namespace detail

using detail::common_reference;
using detail::common_reference_t;

template<typename From, typename To>
using convertible_to =
    lib::to_void<lib::enable_if_t<std::is_convertible<From, To>{}>, decltype(static_cast<To>(std::declval<From>()))>;

template<typename From, typename To>
struct is_convertible_to : lib::is_detected<lib::convertible_to, From, To> {};

template<typename T, typename U>
using same_as = lib::enable_if_t<std::is_same<T, U>{}>;

template<typename From, typename To>
struct is_same_as : lib::is_detected<lib::same_as, From, To> {};

template<typename T, typename U>
using common_reference_with = lib::to_void<
    lib::same_as<lib::common_reference_t<T, U>, lib::common_reference_t<U, T>>,
    lib::convertible_to<T, lib::common_reference_t<T, U>>,
    lib::convertible_to<U, lib::common_reference_t<T, U>>>;

template<typename T, typename U>
struct is_common_reference_with : lib::is_detected<lib::common_reference_with, T, U> {};

template<typename T>
using tuple_like = decltype(std::tuple_size<T>::value);

template<typename T>
struct is_tuple_like : lib::is_detected<lib::tuple_like, T> {};

template<class T, typename... Args>
using brace_constructible = decltype(T{std::declval<Args>()...});

template<class T, typename... Args>
struct is_brace_constructible : lib::is_detected<lib::brace_constructible, T, Args...> {};

using std::swap;

template<typename T>
using swappable = decltype(swap(std::declval<T &>(), std::declval<T &>()));

template<typename T>
using nothrow_swappable = lib::enable_if_t<noexcept(swap(std::declval<T &>(), std::declval<T &>()))>;

template<typename T>
struct is_swappable : lib::is_detected<lib::swappable, T> {};

template<typename T>
struct is_nothrow_swappable : lib::is_detected<lib::nothrow_swappable, T> {};

template<typename T>
using destructible = lib::enable_if_t<std::is_nothrow_destructible<T>{}>;

template<typename T>
struct is_destructible : lib::is_detected<lib::destructible, T> {};

template<typename T, typename... Args>
using constructible_from = lib::to_void<lib::destructible<T>, lib::enable_if_t<std::is_constructible<T, Args...>{}>>;

template<typename T, typename... Args>
struct is_constructible_from : lib::is_detected<lib::constructible_from, T, Args...> {};

template<typename T>
using move_constructible = lib::to_void<lib::constructible_from<T, T>, lib::convertible_to<T, T>>;

template<typename T>
struct is_move_constructible : lib::is_detected<lib::move_constructible, T> {};

template<typename T, typename U>
using assignable_from = lib::to_void<
    lib::enable_if_t<std::is_lvalue_reference<T>{}>,
    lib::common_reference_with<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>,
    lib::same_as<T, decltype(std::declval<T &>() = std::forward<U>(std::declval<U &>()))>>;

template<typename T, typename U>
struct is_assignable_from : lib::is_detected<lib::assignable_from, T, U> {};

template<typename T>
using movable = lib::to_void<
    lib::enable_if_t<std::is_object<T>{}>,
    lib::move_constructible<T>,
    lib::assignable_from<T &, T>,
    lib::swappable<T>>;

template<typename T>
using is_movable = lib::
    conjunction<std::is_object<T>, std::is_move_constructible<T>, std::is_assignable<T &, T>, lib::is_swappable<T>>;

template<typename T>
using copy_constructible = lib::to_void<
    lib::move_constructible<T>,
    lib::constructible_from<T, T &>,
    lib::convertible_to<T &, T>,
    lib::constructible_from<T, const T &>,
    lib::convertible_to<const T &, T>,
    lib::constructible_from<T, const T>,
    lib::convertible_to<const T, T>>;

template<typename T>
struct is_copy_constructible : lib::is_detected<lib::copy_constructible, T> {};

template<typename T>
using copyable = lib::to_void<
    lib::copy_constructible<T>,
    lib::movable<T>,
    lib::assignable_from<T &, T &>,
    lib::assignable_from<T &, const T &>,
    lib::assignable_from<T &, const T>>;

template<typename T>
using is_copyable = lib::conjunction<
    std::is_copy_constructible<T>,
    lib::is_movable<T>,
    std::is_assignable<T &, T &>,
    std::is_assignable<T &, const T &>,
    std::is_assignable<T &, const T>>;

template<typename B>
using boolean_testable = lib::
    to_void<lib::convertible_to<B, bool>, lib::convertible_to<decltype(!std::forward<B>(std::declval<B &>())), bool>>;

template<typename T>
struct is_boolean_testable : lib::is_detected<lib::boolean_testable, T> {};

template<typename T, typename U>
using weakly_equality_comparable_with = lib::to_void<
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() == std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() == std::declval<const lib::remove_reference_t<T> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() != std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() != std::declval<const lib::remove_reference_t<T> &>())>>;

template<typename T, typename U>
struct is_weakly_equality_comparable_with : lib::is_detected<lib::weakly_equality_comparable_with, T, U> {};

template<typename T>
using equality_comparable = lib::weakly_equality_comparable_with<T, T>;

template<typename T>
struct is_equality_comparable : lib::is_detected<lib::equality_comparable, T> {};

template<typename T, typename U>
using equality_comparable_with = lib::to_void<
    lib::equality_comparable<T>,
    lib::equality_comparable<U>,
    lib::equality_comparable<
        lib::common_reference_t<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>>,
    lib::weakly_equality_comparable_with<T, U>>;

template<typename T, typename U>
struct is_equality_comparable_with : lib::is_detected<lib::equality_comparable_with, T, U> {};

template<typename T, typename U>
using partially_ordered_with = lib::to_void<
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() < std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() > std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() <= std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() >= std::declval<const lib::remove_reference_t<U> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() < std::declval<const lib::remove_reference_t<T> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() > std::declval<const lib::remove_reference_t<T> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() <= std::declval<const lib::remove_reference_t<T> &>())>,
    lib::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() >= std::declval<const lib::remove_reference_t<T> &>())>>;

template<typename T, typename U>
struct is_partially_ordered_with : lib::is_detected<lib::partially_ordered_with, T, U> {};

template<typename T>
using totally_ordered = lib::to_void<lib::equality_comparable<T>, lib::partially_ordered_with<T, T>>;

template<typename T>
struct is_totally_ordered : lib::is_detected<lib::totally_ordered, T> {};

template<typename T, typename U>
using totally_ordered_with = lib::to_void<
    lib::totally_ordered<T>,
    lib::totally_ordered<U>,
    lib::equality_comparable_with<T, U>,
    lib::totally_ordered<
        lib::common_reference_t<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>>,
    lib::partially_ordered_with<T, U>>;

template<typename T, typename U>
struct is_totally_ordered_with : lib::is_detected<lib::totally_ordered_with, T, U> {};

template<typename T>
using hashable = lib::same_as<decltype(std::declval<std::hash<T>>()(std::declval<T &>())), std::size_t>;

template<typename T>
struct is_hashable : lib::is_detected<lib::hashable, T> {};

template<typename T, typename Stream>
using printable = lib::same_as<decltype(std::declval<Stream &>() << std::declval<const T &>()), Stream &>;

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_printable : lib::is_detected<lib::printable, T, std::basic_ostream<CharT, Traits>> {};

template<typename T, typename Stream>
using parsable = lib::same_as<decltype(std::declval<Stream &>() >> std::declval<T &>()), Stream &>;

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_parsable : lib::is_detected<lib::parsable, T, std::basic_istream<CharT, Traits>> {};

template<typename, template<typename...> class>
struct is_specialization_of : std::false_type {};

template<typename... Args, template<typename...> class F>
struct is_specialization_of<F<Args...>, F> : std::true_type {};

template<typename T, template<typename...> class F>
using specialization_of = lib::enable_if_t<lib::is_specialization_of<T, F>{}>;

template<typename T>
using default_initializable =
    lib::to_void<lib::enable_if_t<lib::is_constructible_from<T>{}>, decltype(T{}), decltype(::new T)>;

template<typename T>
struct is_default_initializable : lib::is_detected<lib::default_initializable, T> {};

template<typename T>
using semiregular = lib::to_void<lib::copyable<T>, lib::default_initializable<T>>;

template<typename T>
struct is_semiregular : lib::is_detected<lib::semiregular, T> {};

template<typename T>
using regular = lib::to_void<lib::semiregular<T>, lib::equality_comparable<T>>;

template<typename T>
struct is_regular : lib::is_detected<lib::regular, T> {};

template<typename T>
using integral = lib::enable_if_t<std::is_integral<T>{}>;

using std::is_integral;

template<typename T>
using signed_integral = lib::enable_if_t<std::is_integral<T>() && std::is_signed<T>()>;

template<typename T>
struct is_signed_integral : lib::is_detected<lib::signed_integral, T> {};

template<typename T>
using unsigned_integral = lib::enable_if_t<std::is_integral<T>() && std::is_unsigned<T>()>;

template<typename T>
struct is_unsigned_integral : lib::is_detected<lib::unsigned_integral, T> {};

template<typename Derived, typename Base>
using derived_from = lib::enable_if_t<
    std::is_base_of<Base, Derived>() && std::is_convertible<const volatile Derived *, const volatile Base *>()>;

template<typename Derived, typename Base>
struct is_derived_from : lib::is_detected<lib::derived_from, Derived, Base> {};

template<typename T>
struct is_referenceable
    : std::integral_constant<
          bool,
          std::is_object<T>() ||
              (std::is_function<T>() && !std::is_const<T>() && !std::is_volatile<T>() && !std::is_reference<T>()) ||
              std::is_reference<T>()> {};

template<typename T>
using referenceable = lib::enable_if_t<lib::is_referenceable<T>{}>;

template<typename T>
using dereferenceable = lib::referenceable<decltype(*std::declval<T &>())>;

template<typename T>
struct is_dereferenceable : lib::is_detected<lib::dereferenceable, T> {};

}} // namespace halcheck::lib

#endif
