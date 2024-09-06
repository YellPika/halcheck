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

static const struct optional_t : gen::labelable<optional_t> {
  using gen::labelable<optional_t>::operator();

  /// @brief Generates a std::optional.
  /// @tparam F The type of generator for producing non-std::nullopt elements.
  /// @param gen A generator for producing non-std::nullopt elements.
  /// @return A random std::optional.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  lib::optional<lib::invoke_result_t<F>> operator()(F gen) const {
    using namespace lib::literals;

    if (gen::sample(1, gen::size()) && !gen::shrink())
      return gen::label("gen"_s, std::move(gen));
    else
      return lib::nullopt;
  }
} optional;

}} // namespace halcheck::gen

#endif
