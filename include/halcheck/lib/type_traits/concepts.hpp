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

namespace concepts {
template<typename From, typename To>
using convertible_to =
    lib::to_void<lib::enable_if_t<std::is_convertible<From, To>{}>, decltype(static_cast<To>(std::declval<From>()))>;
} // namespace concepts

template<typename From, typename To>
struct is_convertible_to : lib::is_detected<lib::concepts::convertible_to, From, To> {};

namespace concepts {
template<typename T, typename U>
using same_as = lib::enable_if_t<std::is_same<T, U>{}>;
} // namespace concepts

template<typename From, typename To>
struct is_same_as : lib::is_detected<lib::concepts::same_as, From, To> {};

namespace concepts {
template<typename T, typename U>
using common_reference_with = lib::to_void<
    concepts::same_as<lib::common_reference_t<T, U>, lib::common_reference_t<U, T>>,
    concepts::convertible_to<T, lib::common_reference_t<T, U>>,
    concepts::convertible_to<U, lib::common_reference_t<T, U>>>;
} // namespace concepts

template<typename T, typename U>
struct is_common_reference_with : lib::is_detected<concepts::common_reference_with, T, U> {};

namespace concepts {
template<typename T>
using tuple_like = decltype(std::tuple_size<T>::value);
} // namespace concepts

template<typename T>
struct is_tuple_like : lib::is_detected<concepts::tuple_like, T> {};

namespace concepts {
template<class T, typename... Args>
using brace_constructible = decltype(T{std::declval<Args>()...});
} // namespace concepts

template<class T, typename... Args>
struct is_brace_constructible : lib::is_detected<concepts::brace_constructible, T, Args...> {};

namespace concepts {
using std::swap;

template<typename T>
using swappable = decltype(swap(std::declval<T &>(), std::declval<T &>()));

template<typename T>
using nothrow_swappable = lib::enable_if_t<noexcept(swap(std::declval<T &>(), std::declval<T &>()))>;
} // namespace concepts

template<typename T>
struct is_swappable : lib::is_detected<concepts::swappable, T> {};

template<typename T>
struct is_nothrow_swappable : lib::is_detected<concepts::nothrow_swappable, T> {};

namespace concepts {
template<typename T>
using destructible = lib::enable_if_t<std::is_nothrow_destructible<T>{}>;
} // namespace concepts

template<typename T>
struct is_destructible : lib::is_detected<concepts::destructible, T> {};

namespace concepts {
template<typename T, typename... Args>
using constructible_from =
    lib::to_void<concepts::destructible<T>, lib::enable_if_t<std::is_constructible<T, Args...>{}>>;
} // namespace concepts

template<typename T, typename... Args>
struct is_constructible_from : lib::is_detected<concepts::constructible_from, T, Args...> {};

namespace concepts {
template<typename T>
using move_constructible = lib::to_void<concepts::constructible_from<T, T>, concepts::convertible_to<T, T>>;
} // namespace concepts

template<typename T>
struct is_move_constructible : lib::is_detected<concepts::move_constructible, T> {};

namespace concepts {
template<typename T, typename U>
using assignable_from = lib::to_void<
    lib::enable_if_t<std::is_lvalue_reference<T>{}>,
    concepts::common_reference_with<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>,
    concepts::same_as<T, decltype(std::declval<T &>() = std::forward<U>(std::declval<U &>()))>>;
} // namespace concepts

template<typename T, typename U>
struct is_assignable_from : lib::is_detected<concepts::assignable_from, T, U> {};

namespace concepts {
template<typename T>
using movable = lib::to_void<
    lib::enable_if_t<std::is_object<T>{}>,
    concepts::move_constructible<T>,
    concepts::assignable_from<T &, T>,
    concepts::swappable<T>>;
} // namespace concepts

template<typename T>
using is_movable = lib::
    conjunction<std::is_object<T>, std::is_move_constructible<T>, std::is_assignable<T &, T>, lib::is_swappable<T>>;

namespace concepts {
template<typename T>
using copy_constructible = lib::to_void<
    concepts::move_constructible<T>,
    concepts::constructible_from<T, T &>,
    concepts::convertible_to<T &, T>,
    concepts::constructible_from<T, const T &>,
    concepts::convertible_to<const T &, T>,
    concepts::constructible_from<T, const T>,
    concepts::convertible_to<const T, T>>;
} // namespace concepts

template<typename T>
struct is_copy_constructible : lib::is_detected<concepts::copy_constructible, T> {};

namespace concepts {
template<typename T>
using copyable = lib::to_void<
    concepts::copy_constructible<T>,
    concepts::movable<T>,
    concepts::assignable_from<T &, T &>,
    concepts::assignable_from<T &, const T &>,
    concepts::assignable_from<T &, const T>>;
} // namespace concepts

template<typename T>
using is_copyable = lib::conjunction<
    std::is_copy_constructible<T>,
    lib::is_movable<T>,
    std::is_assignable<T &, T &>,
    std::is_assignable<T &, const T &>,
    std::is_assignable<T &, const T>>;

namespace concepts {

template<typename B>
using boolean_testable = lib::to_void<
    concepts::convertible_to<B, bool>,
    concepts::convertible_to<decltype(!std::forward<B>(std::declval<B &>())), bool>>;
} // namespace concepts

template<typename T>
struct is_boolean_testable : lib::is_detected<concepts::boolean_testable, T> {};

namespace concepts {
template<typename T, typename U>
using weakly_equality_comparable_with = lib::to_void<
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() == std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() == std::declval<const lib::remove_reference_t<T> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() != std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() != std::declval<const lib::remove_reference_t<T> &>())>>;
} // namespace concepts

template<typename T, typename U>
struct is_weakly_equality_comparable_with : lib::is_detected<concepts::weakly_equality_comparable_with, T, U> {};

namespace concepts {
template<typename T>
using equality_comparable = concepts::weakly_equality_comparable_with<T, T>;
} // namespace concepts

template<typename T>
struct is_equality_comparable : lib::is_detected<concepts::equality_comparable, T> {};

namespace concepts {
template<typename T, typename U>
using equality_comparable_with = lib::to_void<
    concepts::equality_comparable<T>,
    concepts::equality_comparable<U>,
    concepts::equality_comparable<
        lib::common_reference_t<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>>,
    concepts::weakly_equality_comparable_with<T, U>>;
} // namespace concepts

template<typename T, typename U>
struct is_equality_comparable_with : lib::is_detected<concepts::equality_comparable_with, T, U> {};

namespace concepts {
template<typename T, typename U>
using partially_ordered_with = lib::to_void<
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() < std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() > std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() <= std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<T> &>() >= std::declval<const lib::remove_reference_t<U> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() < std::declval<const lib::remove_reference_t<T> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() > std::declval<const lib::remove_reference_t<T> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() <= std::declval<const lib::remove_reference_t<T> &>())>,
    concepts::boolean_testable<
        decltype(std::declval<const lib::remove_reference_t<U> &>() >= std::declval<const lib::remove_reference_t<T> &>())>>;
} // namespace concepts

template<typename T, typename U>
struct is_partially_ordered_with : lib::is_detected<concepts::partially_ordered_with, T, U> {};

namespace concepts {
template<typename T>
using totally_ordered = lib::to_void<concepts::equality_comparable<T>, concepts::partially_ordered_with<T, T>>;
} // namespace concepts

template<typename T>
struct is_totally_ordered : lib::is_detected<concepts::totally_ordered, T> {};

namespace concepts {
template<typename T, typename U>
using totally_ordered_with = lib::to_void<
    concepts::totally_ordered<T>,
    concepts::totally_ordered<U>,
    concepts::equality_comparable_with<T, U>,
    concepts::totally_ordered<
        lib::common_reference_t<const lib::remove_reference_t<T> &, const lib::remove_reference_t<U> &>>,
    concepts::partially_ordered_with<T, U>>;
} // namespace concepts

template<typename T, typename U>
struct is_totally_ordered_with : lib::is_detected<concepts::totally_ordered_with, T, U> {};

namespace concepts {
template<typename T>
using hashable = concepts::same_as<decltype(std::declval<std::hash<T>>()(std::declval<T &>())), std::size_t>;
} // namespace concepts

template<typename T>
struct is_hashable : lib::is_detected<concepts::hashable, T> {};

namespace concepts {
template<typename T, typename Stream>
using printable = concepts::same_as<decltype(std::declval<Stream &>() << std::declval<const T &>()), Stream &>;
} // namespace concepts

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_printable : lib::is_detected<concepts::printable, T, std::basic_ostream<CharT, Traits>> {};

namespace concepts {
template<typename T, typename Stream>
using parsable = concepts::same_as<decltype(std::declval<Stream &>() >> std::declval<T &>()), Stream &>;
} // namespace concepts

template<typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct is_parsable : lib::is_detected<concepts::parsable, T, std::basic_istream<CharT, Traits>> {};

template<typename, template<typename...> class>
struct is_specialization_of : std::false_type {};

template<typename... Args, template<typename...> class F>
struct is_specialization_of<F<Args...>, F> : std::true_type {};

namespace concepts {
template<typename T, template<typename...> class F>
using specialization_of = lib::enable_if_t<lib::is_specialization_of<T, F>{}>;
} // namespace concepts

namespace concepts {
template<typename T>
using default_initializable =
    lib::to_void<lib::enable_if_t<lib::is_constructible_from<T>{}>, decltype(T{}), decltype(::new T)>;
} // namespace concepts

template<typename T>
struct is_default_initializable : lib::is_detected<concepts::default_initializable, T> {};

namespace concepts {
template<typename T>
using semiregular = lib::to_void<concepts::copyable<T>, concepts::default_initializable<T>>;
} // namespace concepts

template<typename T>
struct is_semiregular : lib::is_detected<concepts::semiregular, T> {};

namespace concepts {
template<typename T>
using regular = lib::to_void<concepts::semiregular<T>, concepts::equality_comparable<T>>;
} // namespace concepts

template<typename T>
struct is_regular : lib::is_detected<concepts::regular, T> {};

}} // namespace halcheck::lib

#endif
