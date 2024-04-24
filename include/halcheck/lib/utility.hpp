#ifndef HALCHECK_LIB_UTILITY_HPP
#define HALCHECK_LIB_UTILITY_HPP

#include <halcheck/lib/type_traits.hpp>

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

namespace detail {
template<std::size_t... Ints>
lib::index_sequence<Ints...> downcast(lib::index_sequence<Ints...> seq) {
  return seq;
}

template<typename, std::size_t I, typename, typename>
struct remove_helper;

template<std::size_t I, typename Output>
struct remove_helper<void, I, lib::index_sequence<>, Output> {
  using type = Output;
};

template<std::size_t J, std::size_t... Output, std::size_t I, std::size_t... Input>
struct remove_helper<lib::enable_if_t<I == J>, I, lib::index_sequence<J, Input...>, lib::index_sequence<Output...>>
    : remove_helper<void, I, lib::index_sequence<Input...>, lib::index_sequence<Output...>> {};

template<std::size_t J, std::size_t... Output, std::size_t I, std::size_t... Input>
struct remove_helper<lib::enable_if_t<I != J>, I, lib::index_sequence<J, Input...>, lib::index_sequence<Output...>>
    : remove_helper<void, I, lib::index_sequence<Input...>, lib::index_sequence<Output..., J>> {};
} // namespace detail

template<std::size_t I, typename T>
using remove = typename detail::remove_helper<void, I, decltype(detail::downcast(T())), lib::index_sequence<>>::type;

template<std::size_t... Ints>
constexpr std::size_t sequence_size(lib::index_sequence<Ints...>) {
  return sizeof...(Ints);
}

}} // namespace halcheck::lib

#endif
