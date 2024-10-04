#ifndef HALCHECK_LIB_ITERATOR_INDIRECTLY_READABLE_HPP
#define HALCHECK_LIB_ITERATOR_INDIRECTLY_READABLE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/move.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/iterator/indirectly_readable_traits

namespace detail {
template<typename T, typename = void>
struct cond_value_type {};

template<typename T>
struct cond_value_type<T, lib::enable_if_t<std::is_object<T>{}>> {
  using value_type = T;
};
} // namespace detail

template<typename I, typename>
struct indirectly_readable_traits {};

template<typename I>
struct indirectly_readable_traits<I *> : detail::cond_value_type<I> {};

template<typename I>
struct indirectly_readable_traits<I, lib::enable_if_t<std::is_array<I>{}>> {
  using value_type = lib::remove_cv_t<lib::remove_extent_t<I>>;
};

template<class I>
struct indirectly_readable_traits<const I> : indirectly_readable_traits<I> {};

template<class I>
struct indirectly_readable_traits<
    I,
    lib::enable_if_t<detail::has_member_value_type<I>() && !detail::has_member_element_type<I>()>>
    : detail::cond_value_type<typename I::value_type> {};

template<class I>
struct indirectly_readable_traits<
    I,
    lib::enable_if_t<!detail::has_member_value_type<I>() && detail::has_member_element_type<I>()>>
    : detail::cond_value_type<typename I::element_type> {};

template<class I>
struct indirectly_readable_traits<
    I,
    lib::enable_if_t<
        detail::has_member_value_type<I>() && detail::has_member_element_type<I>() &&
        !std::is_same<detail::member_value_type<I>, detail::member_element_type<I>>()>> {};

template<class I>
struct indirectly_readable_traits<
    I,
    lib::enable_if_t<
        detail::has_member_value_type<I>() && detail::has_member_element_type<I>() &&
        std::is_same<detail::member_value_type<I>, detail::member_element_type<I>>()>>
    : detail::cond_value_type<typename I::element_type> {};

// See https://en.cppreference.com/w/cpp/iterator/indirectly_readable

template<typename I>
using indirectly_readable = lib::to_void<
    lib::iter_value_t<I>,
    lib::iter_reference_t<I>,
    lib::iter_rvalue_reference_t<I>,
    lib::same_as<decltype(*std::declval<const I &>()), lib::iter_reference_t<I>>,
    lib::same_as<decltype(lib::iter_move(std::declval<const I &>())), lib::iter_rvalue_reference_t<I>>,
    lib::common_reference_with<lib::iter_reference_t<I> &&, lib::iter_value_t<I> &>,
    lib::common_reference_with<lib::iter_reference_t<I> &&, lib::iter_rvalue_reference_t<I> &&>,
    lib::common_reference_with<lib::iter_rvalue_reference_t<I> &&, const lib::iter_value_t<I> &>>;

template<typename I>
struct is_indirectly_readable : lib::is_detected<lib::indirectly_readable, lib::remove_cvref_t<I>> {};

// See https://en.cppreference.com/w/cpp/iterator/indirectly_writable

template<typename Out, typename T>
using indirectly_writable = lib::to_void<
    decltype(*std::declval<Out &>() = std::forward<T>(std::declval<T &>())),
    decltype(*std::forward<Out>(std::declval<Out &>()) = std::forward<T>(std::declval<T &>())),
    decltype(const_cast<const lib::iter_reference_t<Out> &&>(*std::declval<Out &>()) = std::forward<T>(std::declval<T &>())),
    decltype(const_cast<const lib::iter_reference_t<Out> &&>(*std::forward<Out>(std::declval<Out &>())) = std::forward<T>(std::declval<T &>()))>;

template<typename Out, typename T>
struct is_indirectly_writable : lib::is_detected<lib::indirectly_writable, Out, T> {};

}} // namespace halcheck::lib

#endif
