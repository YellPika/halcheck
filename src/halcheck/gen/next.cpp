#include "halcheck/gen/next.hpp"

#include <halcheck/ext/doctest.hpp>

#include <stdexcept>
#include <utility>

using namespace halcheck;

const gen::next_t gen::next([](const gen::weight &, const gen::weight &) -> bool {
  throw std::runtime_error("halcheck::gen::next: no handler provided");
});

const lib::effect<gen::weight> gen::size([] { return 0; });

TEST_SUITE("halcheck::source") {
  TEST_CASE("gen::next throws by default") { CHECK_THROWS(gen::next()); }

  TEST_CASE("gen::next.handle works with all kinds of functions") {
    auto strategy = [](const gen::weight &, const gen::weight &) { return true; };
    gen::next.handle(strategy);
    gen::next.handle(std::move(strategy));
  }
}
