#ifndef HALCHECK_GEN_VARIANT_HPP
#define HALCHECK_GEN_VARIANT_HPP

#include <halcheck/gen/element.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
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
  lib::variant<lib::invoke_result_t<Fs, lib::atom>...> operator()(T i) {
    using namespace lib::literals;
    static constexpr auto I = decltype(i)::value;
    return lib::variant<lib::invoke_result_t<Fs, lib::atom>...>(
        lib::in_place_index_t<I>(),
        lib::invoke(std::get<I>(tuple), "gen"_s));
  }
  explicit variant_visitor(Fs... gens) : tuple(std::move(gens)...) {}
  std::tuple<Fs...> tuple;
};
} // namespace detail

/// @brief Generates a random std::integral_constant from the given set of
/// indices.
/// @tparam Ints The set of indices to pick from.
/// @return A random std::integral_constant.
template<std::size_t... Ints, HALCHECK_REQUIRE(sizeof...(Ints) > 0)>
lib::variant<std::integral_constant<std::size_t, Ints>...> index(lib::atom id, lib::index_sequence<Ints...> = {}) {
  using T = lib::variant<std::integral_constant<std::size_t, Ints>...>;
  return gen::element(id, T(std::integral_constant<std::size_t, Ints>())...);
}

/// @brief Constructs a random variant.
/// @tparam Fs The generator function types for each variant element type.
/// @param gens The generators for each variant element type.
/// @return A variant whose value is drawn from a randomly selected element of
/// gens.
template<typename... Fs, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Fs, lib::atom>...>())>
lib::variant<lib::invoke_result_t<Fs, lib::atom>...> variant(lib::atom id, Fs... gens) {
  using namespace lib::literals;
  auto _ = gen::label(id);
  auto i = gen::index("index"_s, lib::make_index_sequence<sizeof...(Fs)>{});
  return lib::visit(detail::variant_visitor<Fs...>(std::move(gens)...), i);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable_r<T, Args, lib::atom>...>()),
    HALCHECK_REQUIRE(!std::is_void<T>())>
T one(lib::atom id, lib::in_place_type_t<T>, Args... args) {
  return lib::visit([](T output) { return output; }, gen::variant(id, std::move(args)...));
}

template<typename... Args, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Args, lib::atom>...>())>
void one(lib::atom id, lib::in_place_type_t<void>, Args... args) {
  gen::one(id, lib::in_place_type_t<int>(), [&](lib::atom id) {
    lib::invoke(args, id);
    return 0;
  }...);
}

template<typename... Args, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Args, lib::atom>...>())>
lib::common_type_t<lib::invoke_result_t<Args, lib::atom>...> one(lib::atom id, Args... args) {
  return gen::one(
      id,
      lib::in_place_type_t<lib::common_type_t<lib::invoke_result_t<Args, lib::atom>...>>(),
      std::move(args)...);
}

}} // namespace halcheck::gen

#endif
