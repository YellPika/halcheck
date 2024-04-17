#include "halcheck/lib/effect.hpp"

#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/element.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

using namespace halcheck;

thread_local std::vector<void *> lib::context::state;
std::atomic_size_t lib::context::next{0};
