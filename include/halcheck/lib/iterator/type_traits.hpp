#ifndef HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP
#define HALCHECK_LIB_ITERATOR_TYPE_TRAITS_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/iterator/input_or_output_iterator

template<typename I, typename D = typename std::iterator_traits<I>::difference_type>
using iterator = lib::to_void<
    lib::copy_constructible<I>,
    lib::copy_assignable<I>,
    lib::destructible<I>,
    lib::swappable<I>,
    lib::enable_if_t<(std::is_integral<D>() && std::is_signed<D>()) || std::is_void<D>()>,
    decltype(*std::declval<I &>()),
    lib::same<decltype(++std::declval<I &>()), I &>>;

template<typename I>
struct is_iterator : lib::is_detected<lib::iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/input_iterator

template<
    typename I,
    typename T = typename std::iterator_traits<I>::value_type,
    typename R = typename std::iterator_traits<I>::reference>
using input_iterator = lib::to_void<
    lib::iterator<I>,
    lib::equality_comparable<I>,
    lib::same<decltype(*std::declval<I &>()), R>,
    lib::convertible<R, T>,
    lib::convertible<decltype(*std::declval<I &>()++), T>>;

template<typename I>
struct is_input_iterator : lib::is_detected<lib::input_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/output_iterator

template<typename I, typename T>
using output_iterator = lib::to_void<
    lib::iterator<I>,
    decltype(*std::declval<I &>() = std::forward<T>(std::declval<T &>())),
    lib::convertible<decltype(std::declval<I &>()++), const I &>,
    decltype(*std::declval<I &>()++ = std::forward<T>(std::declval<T &>()))>;

template<typename I, typename T>
struct is_output_iterator : lib::is_detected<lib::output_iterator, I, T> {};

// See https://en.cppreference.com/w/cpp/iterator/forward_iterator

template<typename I, typename R = typename std::iterator_traits<I>::reference>
using forward_iterator = lib::to_void<
    lib::input_iterator<I>,
    lib::default_constructible<I>,
    lib::convertible<decltype(std::declval<I &>()++), const I &>,
    lib::same<decltype(*std::declval<I &>()++), R>>;

template<typename I>
struct is_forward_iterator : lib::is_detected<lib::forward_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/bidirectional_iterator

template<typename I, typename R = typename std::iterator_traits<I>::reference>
using bidirectional_iterator = lib::to_void<
    lib::forward_iterator<I>,
    lib::same<decltype(*std::declval<I &>()--), R>,
    lib::same<decltype(--std::declval<I &>()), I &>,
    lib::convertible<decltype(std::declval<I &>()--), const I &>>;

template<typename I>
struct is_bidirectional_iterator : lib::is_detected<lib::bidirectional_iterator, I> {};

// See https://en.cppreference.com/w/cpp/iterator/random_access_iterator

template<
    typename I,
    typename N = typename std::iterator_traits<I>::difference_type,
    typename R = typename std::iterator_traits<I>::reference>
using random_access_iterator = lib::to_void<
    lib::bidirectional_iterator<I>,
    lib::same<decltype(std::declval<I &>() += std::declval<const N &>()), I &>,
    lib::same<decltype(std::declval<const N &>() + std::declval<const I &>()), I>,
    lib::same<decltype(std::declval<const I &>() + std::declval<const N &>()), I>,
    lib::same<decltype(std::declval<I &>() -= std::declval<const N &>()), I &>,
    lib::same<decltype(std::declval<const I &>() - std::declval<const N &>()), I>,
    lib::same<decltype(std::declval<const I &>() - std::declval<const I &>()), N>,
    lib::same<decltype(std::declval<const I &>()[std::declval<const N &>()]), R>,
    lib::boolean_testable<decltype(std::declval<const I &>() < std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() > std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() <= std::declval<const I &>())>,
    lib::boolean_testable<decltype(std::declval<const I &>() >= std::declval<const I &>())>>;

template<typename I>
struct is_random_access_iterator : lib::is_detected<lib::random_access_iterator, I> {};

}} // namespace halcheck::lib

#endif
