#ifndef HALCHECK_TYCHE_OBSERVE_HPP
#define HALCHECK_TYCHE_OBSERVE_HPP

#include <halcheck/lib/string.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/strategy.hpp>

namespace halcheck { namespace tyche {

test::strategy
observe(std::string name, std::string folder = lib::getenv("HALCHECK_FOLDER").value_or(".halcheck/observe"));

}} // namespace halcheck::tyche

#endif
