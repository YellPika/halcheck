#ifndef HALCHECK_GEN_OPTIONAL_HPP
#define HALCHECK_GEN_OPTIONAL_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace gen {

/// @brief Generates a std::optional.
/// @tparam F The type of generator for producing non-std::nullopt elements.
/// @param gen A generator for producing non-std::nullopt elements.
/// @return A random std::optional.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
lib::optional<lib::invoke_result_t<F>> optional(F gen) {
  auto _ = gen::group();
  if (gen::next(1, gen::size()) && !gen::shrink())
    return lib::invoke(std::move(gen));
  else
    return lib::nullopt;
}

}} // namespace halcheck::gen

#endif
