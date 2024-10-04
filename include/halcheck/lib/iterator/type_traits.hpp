#ifndef HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP
#define HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/incrementable.hpp>
#include <halcheck/lib/iterator/indirectly_readable.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/iterator/input_or_output_iterator

template<typename I>
using input_or_output_iterator =
    lib::to_void<lib::enable_if_t<lib::is_referenceable<I>{}>, lib::weakly_incrementable<I>>;

template<typename I>
struct is_input_or_output_iterator : lib::is_detected<lib::input_or_output_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/sentinel_for

template<typename S, typename I>
using sentinel_for =
    lib::to_void<lib::semiregular<S>, lib::input_or_output_iterator<I>, lib::weakly_equality_comparable_with<S, I>>;

template<typename S, typename I>
struct is_sentinel_for : lib::is_detected<lib::sentinel_for, S, I> {};

template<class S, class I>
struct disable_sized_sentinel_for : std::false_type {};

template<typename S, typename I>
using sized_sentinel_for = lib::to_void<
    lib::sentinel_for<S, I>,
    lib::enable_if_t<!lib::disable_sized_sentinel_for<lib::remove_cv_t<S>, lib::remove_cv_t<I>>{}>,
    lib::same_as<decltype(std::declval<const S &>() - std::declval<const I &>()), lib::iter_difference_t<I>>,
    lib::same_as<decltype(std::declval<const I &>() - std::declval<const S &>()), lib::iter_difference_t<I>>>;

template<typename S, typename I>
struct is_sized_sentinel_for : lib::is_detected<lib::sized_sentinel_for, S, I> {};

// See https://en.cppreference.com/w/cpp/iterator/input_iterator

template<typename I>
using input_iterator = lib::to_void<
    lib::input_or_output_iterator<I>,
    lib::indirectly_readable<I>,
    lib::derived_from<lib::iter_concept_t<I>, std::input_iterator_tag>>;

template<typename I>
struct is_input_iterator : lib::is_detected<lib::input_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/output_iterator

template<typename I, typename T>
using output_iterator = lib::to_void<
    lib::input_or_output_iterator<I>,
    lib::indirectly_writable<I, T>,
    decltype(*std::declval<I &>() = std::forward<T>(std::declval<T &>()))>;

template<typename I, typename T>
struct is_output_iterator : lib::is_detected<lib::output_iterator, I, T> {};

// See https://en.cppreference.com/w/cpp/iterator/forward_iterator

template<typename I>
using forward_iterator = lib::to_void<
    lib::input_iterator<I>,
    lib::derived_from<lib::iter_concept_t<I>, std::forward_iterator_tag>,
    lib::incrementable<I>,
    lib::sentinel_for<I, I>>;

template<typename I>
struct is_forward_iterator : lib::is_detected<lib::forward_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/bidirectional_iterator

template<typename I>
using bidirectional_iterator = lib::to_void<
    lib::forward_iterator<I>,
    lib::derived_from<lib::iter_concept_t<I>, std::bidirectional_iterator_tag>,
    lib::same_as<decltype(--std::declval<I &>()), I &>,
    lib::same_as<decltype(std::declval<I &>()--), I>>;

template<typename I>
struct is_bidirectional_iterator : lib::is_detected<lib::bidirectional_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/random_access_iterator

template<typename I>
using random_access_iterator = lib::to_void<
    lib::bidirectional_iterator<I>,
    lib::derived_from<lib::iter_concept_t<I>, std::random_access_iterator_tag>,
    lib::totally_ordered<I>,
    lib::sized_sentinel_for<I, I>,
    lib::same_as<decltype(std::declval<I &>() += std::declval<const lib::iter_difference_t<I> &>()), I &>,
    lib::same_as<decltype(std::declval<const lib::iter_difference_t<I> &>() + std::declval<const I &>()), I>,
    lib::same_as<decltype(std::declval<const I &>() + std::declval<const lib::iter_difference_t<I> &>()), I>,
    lib::same_as<decltype(std::declval<I &>() -= std::declval<const lib::iter_difference_t<I> &>()), I &>,
    lib::same_as<decltype(std::declval<const I &>() - std::declval<const lib::iter_difference_t<I> &>()), I>,
    lib::same_as<
        decltype(std::declval<const I &>()[std::declval<const lib::iter_difference_t<I> &>()]),
        lib::iter_reference_t<I>>>;

template<typename I>
struct is_random_access_iterator : lib::is_detected<lib::random_access_iterator, I> {};

}} // namespace halcheck::lib

#endif
