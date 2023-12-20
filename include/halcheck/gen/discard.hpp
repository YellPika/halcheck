#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/raise.hpp>

namespace halcheck { namespace gen {

/// @brief Calls gen::discard if the condition is false.
/// @param The condition to check.
void guard(bool);

/// @brief Exits a property without indicating failure.
extern lib::effect<lib::raise> discard;

}} // namespace halcheck::gen

#endif
