#ifndef HALCHECK_LIB_ITERATOR_BASE_HPP
#define HALCHECK_LIB_ITERATOR_BASE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <iterator>

namespace halcheck { namespace lib {

template<typename I>
using iter_value_t = typename std::iterator_traits<I>::value_type;

template<typename I>
using iter_reference_t = typename std::iterator_traits<I>::reference;

template<typename I>
using iter_difference_t = typename std::iterator_traits<I>::difference_type;

template<typename I>
using iter_category_t = typename std::iterator_traits<I>::iterator_category;

}} // namespace halcheck::lib

#endif
