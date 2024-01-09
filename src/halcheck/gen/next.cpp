#include "halcheck/gen/next.hpp"

#include <stdexcept>
#include <utility>

using namespace halcheck;

const gen::next_t gen::next([](const gen::weight &, const gen::weight &) -> bool {
  throw std::runtime_error("halcheck::gen::next: no handler provided");
});

const lib::effect<gen::weight> gen::size([] { return 0; });
