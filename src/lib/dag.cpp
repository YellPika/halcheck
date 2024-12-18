#include "halcheck/lib/dag.hpp"

#include <halcheck/lib/iterator.hpp>

using namespace halcheck;

using iterator = lib::iterator_t<lib::dag<int>>;
using const_iterator = lib::iterator_t<const lib::dag<int>>;
static_assert(lib::is_random_access_iterator<iterator>::value, "");
static_assert(lib::is_random_access_iterator<const_iterator>::value, "");
