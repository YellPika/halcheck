#ifndef HALCHECK_LIB_ITERATOR_BASE_HPP
#define HALCHECK_LIB_ITERATOR_BASE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <iterator>

namespace halcheck { namespace lib {

/**
 * @brief The type of value pointed to by an iterator.
 * @tparam I The type of iterator to query.
 * @ingroup lib-iterator
 */
template<typename I>
using iter_value_t = typename std::iterator_traits<I>::value_type;

/**
 * @brief The return type of `operator*` for an iterator.
 * @tparam I The type of iterator to query.
 * @ingroup lib-iterator
 */
template<typename I>
using iter_reference_t = typename std::iterator_traits<I>::reference;

/**
 * @brief The return type of `operator-` for an iterator.
 * @tparam I The type of iterator to query.
 * @ingroup lib-iterator
 */
template<typename I>
using iter_difference_t = typename std::iterator_traits<I>::difference_type;

/**
 * @brief A tag type indicating the level of supported iterator options a type provides.
 * @tparam I The type of iterator to query.
 * @ingroup lib-iterator
 */
template<typename I>
using iter_category_t = typename std::iterator_traits<I>::iterator_category;

}} // namespace halcheck::lib

#endif
