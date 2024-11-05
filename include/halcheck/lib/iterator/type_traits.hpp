#ifndef HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP
#define HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

/** @private */
template<typename I>
using iterator = lib::void_t<
    lib::copy_constructible<I>,
    lib::copy_assignable<I>,
    lib::destructible<I>,
    lib::swappable<I>,
    lib::enable_if_t<
        (std::is_integral<lib::iter_difference_t<I>>() && std::is_signed<lib::iter_difference_t<I>>()) ||
        std::is_void<lib::iter_difference_t<I>>()>,
    decltype(*std::declval<I &>()),
    lib::same<decltype(++std::declval<I &>()), I &>>;

/**
 * @brief Determines whether a type satisfies the [LegacyIterator](https://en.cppreference.com/w/cpp/named_req/Iterator)
 * concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/Iterator
 * @ingroup lib-iterator
 */
template<typename I>
struct is_iterator : lib::is_detected<lib::iterator, I> {};

/** @private */
template<typename I>
using input_iterator = lib::void_t<
    lib::iterator<I>,
    lib::equality_comparable<I>,
    lib::same<decltype(*std::declval<I &>()), lib::iter_reference_t<I>>,
    lib::convertible<lib::iter_reference_t<I>, lib::iter_value_t<I>>,
    lib::convertible<decltype(*std::declval<I &>()++), lib::iter_value_t<I>>,
    lib::base_of<std::input_iterator_tag, lib::iter_category_t<I>>>;

/**
 * @brief Determines whether a type satisfies the
 * [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/InputIterator
 * @ingroup lib-iterator
 */
template<typename I>
struct is_input_iterator : lib::is_detected<lib::input_iterator, I> {};

/** @private */
template<typename I, typename T>
using output_iterator = lib::void_t<
    lib::iterator<I>,
    decltype(*std::declval<I &>() = std::forward<T>(std::declval<T &>())),
    lib::convertible<decltype(std::declval<I &>()++), const I &>,
    decltype(*std::declval<I &>()++ = std::forward<T>(std::declval<T &>())),
    lib::base_of<std::output_iterator_tag, lib::iter_category_t<I>>>;

/**
 * @brief Determines whether a type satisfies the
 * [LegacyOutputIterator](https://en.cppreference.com/w/cpp/named_req/OutputIterator) concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/OutputIterator
 * @ingroup lib-iterator
 */
template<typename I, typename T>
struct is_output_iterator : lib::is_detected<lib::output_iterator, I, T> {};

/** @private */
template<typename I>
using forward_iterator = lib::void_t<
    lib::input_iterator<I>,
    lib::default_constructible<I>,
    lib::convertible<decltype(std::declval<I &>()++), const I &>,
    lib::same<decltype(*std::declval<I &>()++), lib::iter_reference_t<I>>,
    lib::base_of<std::forward_iterator_tag, lib::iter_category_t<I>>>;

/**
 * @brief Determines whether a type satisfies the
 * [LegacyForwardIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator) concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/ForwardIterator
 * @ingroup lib-iterator
 */
template<typename I>
struct is_forward_iterator : lib::is_detected<lib::forward_iterator, I> {};

/** @private */
template<typename I>
using bidirectional_iterator = lib::void_t<
    lib::forward_iterator<I>,
    lib::same<decltype(*std::declval<I &>()--), lib::iter_reference_t<I>>,
    lib::same<decltype(--std::declval<I &>()), I &>,
    lib::convertible<decltype(std::declval<I &>()--), const I &>,
    lib::base_of<std::bidirectional_iterator_tag, lib::iter_category_t<I>>>;

/**
 * @brief Determines whether a type satisfies the
 * [LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator) concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator
 * @ingroup lib-iterator
 */
template<typename I>
struct is_bidirectional_iterator : lib::is_detected<lib::bidirectional_iterator, I> {};

/** @private */
template<typename I>
using random_access_iterator = lib::void_t<
    lib::bidirectional_iterator<I>,
    lib::same<decltype(std::declval<I &>() += std::declval<const lib::iter_difference_t<I> &>()), I &>,
    lib::same<decltype(std::declval<const lib::iter_difference_t<I> &>() + std::declval<const I &>()), I>,
    lib::same<decltype(std::declval<const I &>() + std::declval<const lib::iter_difference_t<I> &>()), I>,
    lib::same<decltype(std::declval<I &>() -= std::declval<const lib::iter_difference_t<I> &>()), I &>,
    lib::same<decltype(std::declval<const I &>() - std::declval<const lib::iter_difference_t<I> &>()), I>,
    lib::same<decltype(std::declval<const I &>() - std::declval<const I &>()), lib::iter_difference_t<I>>,
    lib::convertible<
        decltype(std::declval<const I &>()[std::declval<const lib::iter_difference_t<I> &>()]),
        lib::iter_reference_t<I>>,
    lib::boolean_testable<decltype(std::declval<const I &>() < std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() > std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() <= std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() >= std::declval<const I &>())>,
    lib::base_of<std::random_access_iterator_tag, lib::iter_category_t<I>>>;

/**
 * @brief Determines whether a type satisfies the
 * [LegacyRandomAccessIterator](https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator) concept.
 * @tparam I The type to query.
 * @see https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
 * @ingroup lib-iterator
 */
template<typename I>
struct is_random_access_iterator : lib::is_detected<lib::random_access_iterator, I> {};

}} // namespace halcheck::lib

#endif
