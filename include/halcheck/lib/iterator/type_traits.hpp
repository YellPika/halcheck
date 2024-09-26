#ifndef HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP
#define HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

template<typename I>
using iter_value_t = typename std::iterator_traits<I>::value_type;

template<typename I>
using iter_reference_t = typename std::iterator_traits<I>::reference;

template<typename I>
using iter_pointer_t = typename std::iterator_traits<I>::pointer;

template<typename I>
using iter_difference_t = typename std::iterator_traits<I>::difference_type;

namespace detail {
template<typename I>
using is_iterator_helper = lib::to_void<
    lib::enable_if_t<lib::is_copyable<I>{}>,
    lib::enable_if_t<!std::is_void<decltype(*std::declval<I>())>()>,
    lib::enable_if_t<std::is_same<decltype(++std::declval<I &>()) &, I &>{}>,
    lib::enable_if_t<!std::is_void<decltype(*std::declval<I &>()++)>()>>;
} // namespace detail

/// @brief Tests if a type is an iterator type.
/// @tparam I The type to perform the test on.
template<typename I>
struct is_iterator : lib::is_detected<detail::is_iterator_helper, I> {};

namespace detail {
template<typename I>
using is_input_iterator_helper = lib::to_void<
    lib::enable_if_t<lib::is_iterator<I>{}>,
    lib::enable_if_t<lib::is_equality_comparable<I>{}>,
    lib::enable_if_t<std::is_signed<lib::iter_difference_t<I>>{}>,
    lib::enable_if_t<std::is_same<decltype(++std::declval<I &>()), I &>{}>,
    lib::enable_if_t<std::is_same<decltype(*std::declval<I>()), lib::iter_reference_t<I>>{}>,
    lib::common_type_t<decltype(*std::declval<I>()) &&, lib::iter_value_t<I> &>,
    lib::common_type_t<decltype(*std::declval<I &>()++) &&, lib::iter_value_t<I> &>>;
} // namespace detail

template<typename I>
struct is_input_iterator : lib::is_detected<detail::is_input_iterator_helper, I> {};

namespace detail {
template<typename I>
using is_forward_iterator_helper = lib::to_void<
    lib::enable_if_t<lib::is_input_iterator<I>{}>,
    lib::enable_if_t<std::is_default_constructible<I>{}>,
    lib::enable_if_t<std::is_same<lib::remove_cvref_t<lib::iter_reference_t<I>>, lib::iter_value_t<I>>{}>,
    lib::enable_if_t<std::is_convertible<decltype(std::declval<I &>()++), const I &>{}>,
    lib::enable_if_t<std::is_same<decltype(*std::declval<I &>()++), lib::iter_reference_t<I>>{}>>;
} // namespace detail

template<typename I>
struct is_forward_iterator : lib::is_detected<detail::is_forward_iterator_helper, I> {};

namespace detail {
template<typename I>
using is_bidirectional_iterator_helper = lib::to_void<
    lib::enable_if_t<lib::is_forward_iterator<I>{}>,
    lib::enable_if_t<std::is_same<decltype(--std::declval<I &>()), I &>{}>,
    lib::enable_if_t<std::is_convertible<decltype(std::declval<I &>()--), const I &>{}>,
    lib::enable_if_t<std::is_same<decltype(*std::declval<I &>()--), lib::iter_reference_t<I>>{}>>;
} // namespace detail

template<typename I>
struct is_bidirectional_iterator : lib::is_detected<detail::is_bidirectional_iterator_helper, I> {};

namespace detail {
template<typename I>
using is_random_access_iterator_helper = lib::to_void<
    lib::enable_if_t<lib::is_bidirectional_iterator<I>{}>,
    lib::enable_if_t<lib::is_totally_ordered<I>{}>,
    lib::enable_if_t<std::is_same<decltype(std::declval<I &>() += std::declval<lib::iter_difference_t<I>>()), I &>{}>,
    lib::enable_if_t<std::is_same<decltype(std::declval<const I>() + std::declval<lib::iter_difference_t<I>>()), I>{}>,
    lib::enable_if_t<std::is_same<decltype(std::declval<lib::iter_difference_t<I>>() + std::declval<const I>()), I>{}>,
    lib::enable_if_t<std::is_same<decltype(std::declval<I &>() -= std::declval<lib::iter_difference_t<I>>()), I &>{}>,
    lib::enable_if_t<std::is_same<decltype(std::declval<const I>() - std::declval<lib::iter_difference_t<I>>()), I>{}>,
    lib::enable_if_t<
        std::is_same<decltype(std::declval<const I>() - std::declval<const I>()), lib::iter_difference_t<I>>{}>,
    lib::enable_if_t<std::is_same<
        decltype(std::declval<const I>()[std::declval<lib::iter_difference_t<I>>()]),
        lib::iter_reference_t<I>>{}>>;
} // namespace detail

template<typename I>
struct is_random_access_iterator : lib::is_detected<detail::is_random_access_iterator_helper, I> {};

}} // namespace halcheck::lib

#endif
