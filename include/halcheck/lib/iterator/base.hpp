#ifndef HALCHECK_LIB_ITERATOR_BASE_HPP
#define HALCHECK_LIB_ITERATOR_BASE_HPP

#include <halcheck/lib/iterator/move.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <utility>

namespace halcheck { namespace lib {

namespace detail {
template<typename T>
using member_iterator_concept = typename T::iterator_concept;

template<typename T>
using member_iterator_category = typename T::iterator_category;

template<typename T>
using member_value_type = typename T::value_type;

template<typename T>
struct has_member_value_type : lib::is_detected<detail::member_value_type, T> {};

template<typename T>
using member_element_type = typename T::element_type;

template<typename T>
struct has_member_element_type : lib::is_detected<detail::member_element_type, T> {};

template<typename T>
using member_difference_type = typename T::difference_type;
} // namespace detail

template<typename>
struct is_indirectly_readable;

template<typename, typename = void>
struct indirectly_readable_traits;

template<typename, typename = void>
struct incrementable_traits;

template<typename I>
using iter_value_t = typename lib::conditional_t<
    lib::is_detected<detail::member_value_type, std::iterator_traits<I>>{},
    std::iterator_traits<I>,
    lib::indirectly_readable_traits<I>>::value_type;

template<typename I, HALCHECK_REQUIRE(lib::is_dereferenceable<I>())>
using iter_reference_t = decltype(*std::declval<I &>());

template<typename I>
using iter_const_reference_t = lib::common_reference_t<const lib::iter_value_t<I> &&, lib::iter_reference_t<I>>;

template<typename I>
using iter_difference_t = typename lib::conditional_t<
    lib::is_detected<detail::member_difference_type, std::iterator_traits<I>>{},
    std::iterator_traits<I>,
    lib::incrementable_traits<I>>::difference_type;

template<
    typename I,
    HALCHECK_REQUIRE(lib::is_dereferenceable<I>()),
    typename = decltype(lib::iter_move(std::declval<I &>()))>
using iter_rvalue_reference_t = decltype(lib::iter_move(std::declval<I &>()));

template<typename I>
using iter_category_t = typename lib::conditional_t<
    lib::is_detected<detail::member_iterator_category, std::iterator_traits<I>>{},
    std::iterator_traits<I>,
    I>::iterator_category;

namespace detail {
template<typename T>
struct category_to_concept {
  using iterator_concept = lib::iter_category_t<T>;
};
} // namespace detail

template<typename I>
using iter_concept_t = typename lib::conditional_t<
    lib::is_detected<detail::member_iterator_concept, std::iterator_traits<I>>{},
    std::iterator_traits<I>,
    lib::conditional_t<lib::is_detected<detail::member_iterator_concept, I>{}, I, detail::category_to_concept<I>>>::
    iterator_concept;

template<typename T>
struct indirect_value {
  using type = lib::iter_value_t<T> &;
};

template<typename T>
using indirect_value_t = typename lib::indirect_value<T>::type;

template<typename I, HALCHECK_REQUIRE(lib::is_indirectly_readable<I>())>
using iter_common_reference_t = lib::common_reference_t<lib::iter_reference_t<I>, lib::indirect_value_t<I>>;

}} // namespace halcheck::lib

#endif
