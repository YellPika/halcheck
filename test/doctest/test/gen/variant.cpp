#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <algorithm>
#include <vector>

using namespace halcheck;

TEST_SUITE("gen::variant") {
  HALCHECK_TEST_CASE("gen::variant does not discard if possible") {
    auto _ = gen::discard.handle([] {
      FAIL("discard detected");
      return lib::raise(0);
    });
    gen::variant([&] { return 0; }, [&] { return gen::discard(); }, [&] { return 'a'; });
  }

  HALCHECK_TEST_CASE("gen::variant discards if necessary") {
    struct discard {};
    try {
      auto _ = gen::discard.handle([] { return lib::raise(discard()); });
      gen::variant([&] { return gen::discard(); }, [&] { return gen::discard(); });
    } catch (const discard &) {
    }
  }
}
