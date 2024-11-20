#ifndef HALCHECK_GEN_OPTIONAL_HPP
#define HALCHECK_GEN_OPTIONAL_HPP

/**
 * @defgroup gen-optional gen/optional
 * @brief Generating optional values.
 * @ingroup gen
 */

#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/pp.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace gen {

/**
 * @brief Generates a lib::optional.
 * @tparam F The type of generator for producing non-@ref lib::nullopt elements.
 * @param id A unique identifier for the generated value.
 * @param gen A generator for producing non-@ref lib::nullopt elements.
 * @return A random lib::optional.
 */
HALCHECK_INLINE_CONSTEXPR struct {
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
  lib::optional<lib::invoke_result_t<F, lib::atom>> operator()(lib::atom id, F gen) const {
    using namespace lib::literals;

    auto _ = gen::label(id);
    if (gen::sample("cond"_s, gen::size()) > 0 && !gen::shrink("shrink"_s))
      return lib::invoke(std::move(gen), "gen"_s);
    else
      return lib::nullopt;
  }
} optional;

}} // namespace halcheck::gen

#endif
