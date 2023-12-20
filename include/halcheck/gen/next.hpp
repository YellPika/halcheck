#ifndef HALCHECK_GEN_NEXT_HPP
#define HALCHECK_GEN_NEXT_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/ranges.hpp>

#include <climits>
#include <type_traits>
#include <utility>

namespace halcheck { namespace gen {

extern const struct next_t : lib::effect<bool, const gen::weight &, const gen::weight &> {
  using effect::effect;

  /// @brief Obtains the next bit of randomness.
  /// @param w0 The relative weight for returning false.
  /// @param w1 The relative weight for returning true.
  /// @return An arbitrary boolean. Note that the resulting output distribution is
  ///         not guaranteed to correspond to the given weights. Furthermore,
  ///         passing a weight of zero does not guarantee that the corresponding
  ///         value will not be returned.
  bool operator()(const gen::weight &w0 = 1, const gen::weight &w1 = 1) const { return effect::operator()(w0, w1); }
} next;

}} // namespace halcheck::gen

#endif
