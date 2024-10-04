#ifndef HALCHECK_LIB_ITERATOR_INCREMENTABLE_HPP
#define HALCHECK_LIB_ITERATOR_INCREMENTABLE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/iterator/incrementable_traits

namespace detail {
template<typename I, HALCHECK_REQUIRE(std::is_object<I>())>
std::ptrdiff_t incrementable_type(lib::in_place_type_t<I *>, lib::dispatch_t<3>);

template<typename I>
decltype(incrementable_type(lib::in_place_type_t<I>(), lib::dispatch_t<3>()))
    incrementable_type(lib::in_place_type_t<const I>, lib::dispatch_t<2>);

template<typename I>
typename I::difference_type incrementable_type(lib::in_place_type_t<I>, lib::dispatch_t<1>);

template<
    typename I,
    typename T = decltype(std::declval<const I &>() - std::declval<const I &>()),
    HALCHECK_REQUIRE(lib::is_integral<T>())>
lib::make_signed_t<T> incrementable_type(lib::in_place_type_t<I>, lib::dispatch_t<0>);
} // namespace detail

template<typename I, typename>
struct incrementable_traits {};

template<typename I>
struct incrementable_traits<
    I,
    lib::to_void<decltype(detail::incrementable_type(lib::in_place_type_t<I>(), lib::dispatch_t<3>()))>> {
  using difference_type = decltype(detail::incrementable_type(lib::in_place_type_t<I>(), lib::dispatch_t<3>()));
};

// See https://en.cppreference.com/w/cpp/iterator/weakly_incrementable

template<typename I>
using weakly_incrementable = lib::to_void<
    lib::movable<I>,
    lib::signed_integral<lib::iter_difference_t<I>>,
    lib::same_as<decltype(++std::declval<I &>()), I &>,
    decltype(std::declval<I &>()++)>;

template<typename I>
struct is_weakly_incrementable : lib::is_detected<lib::weakly_incrementable, I> {};

template<typename I>
using weakly_decrementable = lib::to_void<
    lib::weakly_incrementable<I>,
    lib::same_as<decltype(--std::declval<I &>()), I &>,
    decltype(std::declval<I &>()--)>;

template<typename I>
struct is_weakly_decrementable : lib::is_detected<lib::weakly_decrementable, I> {};

// See https://en.cppreference.com/w/cpp/iterator/incrementable

template<typename I>
using incrementable =
    lib::to_void<lib::regular<I>, lib::weakly_incrementable<I>, lib::same_as<decltype(std::declval<I &>()++), I>>;

template<typename I>
struct is_incrementable : lib::is_detected<lib::incrementable, I> {};

template<typename I>
using decrementable =
    lib::to_void<lib::incrementable<I>, lib::weakly_decrementable<I>, lib::same_as<decltype(std::declval<I &>()--), I>>;

template<typename I>
struct is_decrementable : lib::is_detected<lib::decrementable, I> {};

template<typename I, typename N>
using advanceable = lib::to_void<
    lib::decrementable<I>,
    lib::totally_ordered<I>,
    lib::same_as<decltype(std::declval<I &>() += std::declval<N>()), I &>,
    lib::same_as<decltype(std::declval<I &>() -= std::declval<N>()), I &>,
    decltype(I(std::declval<const I &>() + std::declval<N>())),
    decltype(I(std::declval<N>() + std::declval<const I &>())),
    decltype(I(std::declval<const I &>() - std::declval<N>())),
    lib::convertible_to<decltype(std::declval<const I &>() - std::declval<const I &>()), N>>;

template<typename I, typename N>
struct is_advanceable : lib::is_detected<lib::advanceable, I, N> {};

}} // namespace halcheck::lib

#endif
