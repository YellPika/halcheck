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

/// @brief Generates a random std::integral_constant from the given set of
/// indices.
/// @tparam Ints The set of indices to pick from.
/// @return A random std::integral_constant.
template<std::size_t... Ints>
lib::variant<std::integral_constant<std::size_t, Ints>...> index(lib::index_sequence<Ints...> = {}) {
  using T = lib::variant<std::integral_constant<std::size_t, Ints>...>;
  return gen::element({T(std::integral_constant<std::size_t, Ints>())...});
}

namespace detail {

template<typename Seq, typename... Fs>
struct variant_visitor {
  template<std::size_t I>
  lib::variant<lib::invoke_result_t<Fs>...> operator()(std::integral_constant<std::size_t, I>) {
    if (auto result = gen::backtrack(std::get<I>(tuple)))
      return lib::variant<lib::invoke_result_t<Fs>...>(lib::in_place_index_t<I>(), *result);
    else
      return variant_visitor<lib::remove<I, Seq>, Fs...>{std::move(tuple)}.apply();
  }

  template<std::size_t N = 1, HALCHECK_REQUIRE(lib::sequence_size(Seq()) > N)>
  lib::variant<lib::invoke_result_t<Fs>...> apply() {
    return lib::visit(*this, gen::index(Seq()));
  }

  template<std::size_t N = 1, HALCHECK_REQUIRE(lib::sequence_size(Seq()) == N)>
  lib::variant<lib::invoke_result_t<Fs>...> apply() {
    return lib::variant<lib::invoke_result_t<Fs>...>(lib::in_place_index_t<0>(), std::get<0>(tuple)());
  }

  std::tuple<Fs...> tuple;
};

} // namespace detail

/// @brief Constructs a random variant.
/// @tparam Fs The generator function types for each variant element type.
/// @param gens The generators for each variant element type. If any of these
/// generators discard without calling next, then a different generator is
/// attempted.
/// @return A variant whose value is drawn from a randomly selected element of
/// gens.
template<typename... Fs, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Fs>...>())>
lib::variant<lib::invoke_result_t<Fs>...> variant(Fs... gens) {
  auto _ = gen::group();
  return detail::variant_visitor<lib::make_index_sequence<sizeof...(Fs)>, Fs...>{{std::move(gens)...}}.apply();
}

}} // namespace halcheck::gen

#endif
