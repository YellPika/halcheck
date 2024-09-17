#ifndef HALCHECK_GEN_OPTIONAL_HPP
#define HALCHECK_GEN_OPTIONAL_HPP

#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace gen {

/// @brief Generates a std::optional.
/// @tparam F The type of generator for producing non-std::nullopt elements.
/// @param gen A generator for producing non-std::nullopt elements.
/// @return A random std::optional.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
lib::optional<lib::invoke_result_t<F, lib::atom>> optional(lib::atom id, F gen) {
  using namespace lib::literals;

  auto _ = gen::label(id);
  if (gen::sample("cond"_s, 1, gen::size()) && !gen::shrink("shrink"_s))
    return lib::invoke(std::move(gen), "gen"_s);
  else
    return lib::nullopt;
}

}} // namespace halcheck::gen

#endif
