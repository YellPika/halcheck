#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/next.hpp>

using namespace halcheck;

TEST_SUITE("halcheck::source") {
  TEST_CASE("gen::next throws by default") { CHECK_THROWS(gen::next()); }

  TEST_CASE("gen::next.handle works with all kinds of functions") {
    auto strategy = [](const gen::weight &, const gen::weight &) { return true; };
    gen::next.handle(strategy);
    gen::next.handle(std::move(strategy));
  }
}
