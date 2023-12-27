#ifndef HALCHECK_LIB_UTILITY_HPP
#define HALCHECK_LIB_UTILITY_HPP

#include <utility>

namespace halcheck { namespace lib {

#if __cplusplus >= 201304L
using std::index_sequence;
using std::integer_sequence;
using std::make_index_sequence;
using std::make_integer_sequence;
#else
template<typename T, T... Ints>
struct integer_sequence {};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

namespace detail {
template<typename, typename T, T N, T... Ints>
struct make_integer_sequence;

template<typename T, T N, T... Ints>
struct make_integer_sequence<lib::enable_if_t<N == 0>, T, N, Ints...> : integer_sequence<T, Ints...> {};

template<typename T, T N, T... Ints>
struct make_integer_sequence<lib::enable_if_t<N != 0>, T, N, Ints...>
    : make_integer_sequence<void, T, N - 1, N - 1, Ints...> {};

} // namespace detail

template<typename T, T N>
struct make_integer_sequence : detail::make_integer_sequence<void, T, N> {};

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;
#endif

}} // namespace halcheck::lib

#endif
