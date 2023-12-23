#include "halcheck/lib/ranges.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

using namespace halcheck;
using namespace halcheck::lib;

template<typename T>
const T &to_const(T &x) {
  return x;
}

static char array[5];

static_assert(std::is_same<range_value_t<decltype((array))>, char>(), "");
static_assert(std::is_same<range_value_t<decltype(to_const(array))>, char>(), "");
static_assert(std::is_same<range_value_t<std::vector<int>>, int>(), "");
static_assert(std::is_same<range_value_t<std::vector<int> &>, int>(), "");
static_assert(std::is_same<range_value_t<const std::vector<int> &>, int>(), "");
static_assert(std::is_same<range_value_t<std::vector<int> &&>, int>(), "");
static_assert(std::is_same<range_value_t<const std::vector<int> &&>, int>(), "");

static_assert(is_range<decltype((array))>(), "");
static_assert(is_range<decltype(to_const(array))>(), "");
static_assert(is_range<std::vector<int>>(), "");
static_assert(is_range<std::vector<int> &>(), "");
static_assert(is_range<std::vector<int> &&>(), "");
static_assert(is_range<const std::vector<int> &>(), "");
static_assert(is_range<const std::vector<int> &&>(), "");
static_assert(!is_range<int>(), "");

static_assert(is_insertable<std::vector<int>>(), "");
static_assert(!is_insertable<int>(), "");

TEST_CASE("auto_insert_iterator") {
  test::check([] {
    auto actual = gen::arbitrary<std::vector<int>>();
    auto expected = actual;

    auto size = gen::weight::current;
    while (gen::next(1, size))
      expected.push_back(gen::arbitrary<int>());

    auto it1 = expected.begin();
    auto it2 = lib::auto_insert(actual);
    for (; it1 != expected.end(); ++it1, ++it2)
      *it2 = *it1;
    REQUIRE_EQ(actual, expected);
  });
}
