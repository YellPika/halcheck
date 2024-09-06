#ifndef HALCHECK_GEN_VARIANT_HPP
#define HALCHECK_GEN_VARIANT_HPP

#include <halcheck/gen/element.hpp>
#include <halcheck/gen/label.hpp>
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
  lib::variant<lib::invoke_result_t<Fs>...> operator()(T i) {
    using namespace lib::literals;
    static constexpr auto I = decltype(i)::value;
    return lib::variant<lib::invoke_result_t<Fs>...>(
        lib::in_place_index_t<I>(),
        gen::label("gen"_s, std::get<I>(tuple)));
  }
  explicit variant_visitor(Fs... gens) : tuple(std::move(gens)...) {}
  std::tuple<Fs...> tuple;
};
} // namespace detail

static const struct index_t : gen::labelable<index_t> {
  using gen::labelable<index_t>::operator();

  /// @brief Generates a random std::integral_constant from the given set of
  /// indices.
  /// @tparam Ints The set of indices to pick from.
  /// @return A random std::integral_constant.
  template<std::size_t... Ints, HALCHECK_REQUIRE(sizeof...(Ints) > 0)>
  lib::variant<std::integral_constant<std::size_t, Ints>...> operator()(lib::index_sequence<Ints...> = {}) const {
    using T = lib::variant<std::integral_constant<std::size_t, Ints>...>;
    return gen::element(T(std::integral_constant<std::size_t, Ints>())...);
  }
} index;

/// @brief Constructs a random variant.
/// @tparam Fs The generator function types for each variant element type.
/// @param gens The generators for each variant element type.
/// @return A variant whose value is drawn from a randomly selected element of
/// gens.
template<typename... Fs, HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Fs>...>())>
lib::variant<lib::invoke_result_t<Fs>...> variant(Fs... gens) {
  using namespace lib::literals;
  auto i = gen::index("index"_s, lib::make_index_sequence<sizeof...(Fs)>{});
  return lib::visit(detail::variant_visitor<Fs...>(std::move(gens)...), i);
}

static const struct one_t : gen::labelable<one_t> {
  using gen::labelable<one_t>::operator();

  template<
      typename... Args,
      HALCHECK_REQUIRE(!lib::disjunction<std::is_convertible<Args, lib::atom>...>()),
      HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Args>...>())>
  lib::common_type_t<lib::invoke_result_t<Args>...> operator()(Args... args) const {
    return (*this)(lib::in_place_type_t<lib::common_type_t<lib::invoke_result_t<Args>...>>(), std::move(args)...);
  }

  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(!lib::disjunction<std::is_convertible<Args, lib::atom>...>()),
      HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable_r<T, Args>...>()),
      HALCHECK_REQUIRE(!std::is_void<T>())>
  T operator()(lib::in_place_type_t<T>, Args... args) const {
    return lib::visit([](T output) { return output; }, gen::variant(std::move(args)...));
  }

  struct trivial {};

  template<typename F>
  struct wrap {
    template<bool _ = true, HALCHECK_REQUIRE(lib::is_invocable<const F>() && _)>
    trivial operator()() const {
      lib::invoke(func);
      return {};
    }

    trivial operator()() {
      lib::invoke(func);
      return {};
    }

    F func;
  };

  template<
      typename... Args,
      HALCHECK_REQUIRE(!lib::disjunction<std::is_convertible<Args, lib::atom>...>()),
      HALCHECK_REQUIRE(lib::conjunction<lib::is_invocable<Args>...>())>
  void operator()(lib::in_place_type_t<void>, Args... args) const {
    (*this)(lib::in_place_type_t<trivial>(), wrap<Args>{std::move(args)}...);
  }
} one;

}} // namespace halcheck::gen

#endif
