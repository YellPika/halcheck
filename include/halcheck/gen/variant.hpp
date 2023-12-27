#ifndef HALCHECK_GEN_VARIANT_HPP
#define HALCHECK_GEN_VARIANT_HPP

#include <halcheck/gen/element.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace halcheck { namespace gen {

namespace detail {
template<typename... Fs>
struct variant_visitor {
  template<typename T>
  lib::variant<lib::invoke_result_t<Fs>...> operator()(T i) {
    static constexpr auto I = decltype(i)::value;
    return lib::variant<lib::invoke_result_t<Fs>...>(lib::in_place_index_t<I>(), lib::invoke(std::get<I>(tuple)));
  }
  variant_visitor(Fs... gens) : tuple(std::move(gens)...) {}
  std::tuple<Fs...> tuple;
};
} // namespace detail

/// @brief Generates a random std::integral_constant from the given set of
/// indices.
/// @tparam ...Ints The set of indices to pick from.
/// @param  Used for type deduction.
/// @return A random std::integral_constant.
template<std::size_t... Ints>
lib::variant<std::integral_constant<std::size_t, Ints>...> index(lib::index_sequence<Ints...> = {}) {
  using T = lib::variant<std::integral_constant<std::size_t, Ints>...>;
  return gen::element({T(std::integral_constant<std::size_t, Ints>())...});
}

/// @brief Constructs a random variant.
/// @tparam ...Fs The generator function types for each variant element type.
/// @param ...gens The generators for each variant element type.
/// @return A variant whose value is drawn from a randomly selected element of
/// gens.
template<typename... Fs, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Fs>...>())>
lib::variant<lib::invoke_result_t<Fs>...> variant(Fs... gens) {
  auto _ = gen::group();
  return lib::visit(
      detail::variant_visitor<Fs...>(std::move(gens)...), gen::index(lib::make_index_sequence<sizeof...(Fs)>()));
}

}} // namespace halcheck::gen

#endif
