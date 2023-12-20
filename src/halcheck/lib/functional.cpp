#include "halcheck/lib/functional.hpp"

#include <halcheck/lib/type_traits.hpp>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

using namespace halcheck::lib;

static auto lambda0 = [] {};
static auto lambda1 = [](int x) { return x; };

static_assert(is_invocable<decltype(lambda0)>(), "");
static_assert(!is_invocable<decltype(lambda0), int>(), "");
static_assert(!is_invocable<decltype(lambda0), int>(), "");
static_assert(is_invocable<decltype(lambda1), int>(), "");

static_assert(is_invocable_r<void, decltype(lambda0)>(), "");
static_assert(!is_invocable_r<int, decltype(lambda0), int>(), "");
static_assert(!is_invocable_r<void, decltype(lambda0), int>(), "");
static_assert(is_invocable_r<int, decltype(lambda1), int>(), "");
