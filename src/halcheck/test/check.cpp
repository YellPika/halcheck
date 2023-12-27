#include "halcheck/test/check.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/limited.hpp>
#include <halcheck/test/shrinking.hpp>

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace halcheck;

TEST_CASE("check experiment 0") {
  std::size_t total = 0;
  std::map<std::size_t, std::size_t> counts;

  test::check([&] {
    std::size_t sum = 0;
    for (auto &&xs : gen::arbitrary<std::vector<std::string>>())
      sum += xs.size();
    counts[sum]++;
    total++;
  });

  for (auto &&pair : counts)
    std::cout << pair.first << ": " << pair.second << " (" << pair.second * 100.0 / total << "%)\n";
}

TEST_CASE("check experiment 1") {
  test::check([&] {
    std::size_t size = 0;
    std::vector<int> vector;

    auto commands = gen::weight::current;
    while (gen::next(1, commands--)) {
      switch (gen::range(0, 2)) {
      case 0: {
        vector.push_back(gen::arbitrary<int>());
        ++size;
        REQUIRE_EQ(vector.size(), size);
      } break;
      case 1: {
        if (size > 0) {
          vector.pop_back();
          --size;
          REQUIRE_EQ(vector.size(), size);
        }
      } break;
      }
    }
  });
}
