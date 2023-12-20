#include "halcheck/gen/shrink.hpp"

using namespace halcheck;

const gen::shrink_t gen::shrink([](std::uintmax_t) { return lib::nullopt; });
