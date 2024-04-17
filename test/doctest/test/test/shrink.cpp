#include <halcheck/ext/doctest.hpp>
#include <halcheck/fmt/flatten.hpp>
#include <halcheck/fmt/show.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/shrink.hpp>

#include <iostream>
#include <limits>

using namespace halcheck;

HALCHECK_TEST_CASE("shrink test") {
  fmt::flatten flatten(std::clog, 300);
  using T = uint8_t;

  auto threshold = gen::range<T>(0, std::numeric_limits<T>::max());
  CAPTURE(threshold);

  auto size = gen::range<std::size_t>(1, 10);
  CAPTURE(size);

  auto index = gen::range<std::size_t>(0, size);
  CAPTURE(index);

  std::vector<T> expected(size, T(0));
  expected[index] = threshold;
  CAPTURE(expected);

  try {
    test::shrink(test::random())([&] {
      auto xs = gen::arbitrary<std::vector<T>>();
      if (xs.size() >= size && xs[index] >= threshold) {
        std::clog << fmt::show(xs) << std::endl;
        throw xs;
      }
    });

    FAIL("failure not caught!");
  } catch (const std::vector<T> &e) {
    CAPTURE(e);
    CHECK_EQ(e, expected);
  }
}

HALCHECK_TEST_CASE("binary_search test", test::limit(test::random(), 1024)) {
  using limits = std::numeric_limits<int8_t>;
  auto min = gen::range(limits::min(), limits::max());
  auto max = int8_t(gen::range(min, limits::max()) + 1);
  auto err = gen::range(min, max);
  CAPTURE(min);
  CAPTURE(max);
  CAPTURE(err);
  try {
    test::check([&] {
      auto x = gen::range(min, max);
      if (x >= err) {
        MESSAGE(+x, " > ", +err);
        throw x;
      }
    });
  } catch (const decltype(err) &e) {
    CHECK_EQ(e, err);
  }
}
