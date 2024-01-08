#ifndef HALCHECK_GEN_DISCARD_HPP
#define HALCHECK_GEN_DISCARD_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/raise.hpp>

namespace halcheck { namespace gen {

/// @brief Calls gen::discard if the condition is false.
/// @param cond The condition to check.
void guard(bool cond);

/// @brief Exits a test-case without indicating failure.
extern lib::effect<lib::raise> discard;

/// @brief Causes a test to succeed immediately. This function may be a no-op in
///        certain circumstances, such as during shrinking.
extern lib::effect<void> succeed;

}} // namespace halcheck::gen

#endif
