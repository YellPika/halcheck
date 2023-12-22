#include "halcheck/gen/next.hpp"

#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/effect.hpp>

#include <doctest/doctest.h>

#include <climits>
#include <stdexcept>
#include <utility>

using namespace halcheck;

const gen::next_t gen::next([](const gen::weight &, const gen::weight &) -> bool {
  throw std::runtime_error("halcheck::gen::next: no handler provided");
});

TEST_SUITE("halcheck::source") {
  TEST_CASE("gen::next throws by default") { REQUIRE_THROWS(gen::next()); }

  TEST_CASE("gen::next.handle works with all kinds of functions") {
    auto strategy = [](const gen::weight &, const gen::weight &) { return true; };
    gen::next.handle(strategy);
    gen::next.handle(std::move(strategy));
  }
}
