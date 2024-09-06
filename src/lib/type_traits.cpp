#include "halcheck/lib/type_traits.hpp"

#include <tuple>
#include <utility>

using namespace halcheck;

static_assert(lib::is_tuple_like<std::tuple<>>(), "");
static_assert(lib::is_tuple_like<std::pair<int, int>>(), "");
static_assert(!lib::is_tuple_like<int>(), "");
