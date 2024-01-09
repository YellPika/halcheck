#include "halcheck/lib/effect.hpp"

#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/element.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <functional>
#include <vector>

using namespace halcheck;

thread_local std::array<lib::context::frame, lib::context::size> lib::context::stack;
thread_local std::size_t lib::context::top = 0;
std::atomic_size_t lib::context::next{0};
