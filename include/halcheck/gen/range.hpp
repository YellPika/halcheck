#ifndef HALCHECK_GEN_RANGE_HPP
#define HALCHECK_GEN_RANGE_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/numeric.hpp>

#include <climits>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace halcheck { namespace gen {

/// @brief Generates a random integer in a range.
/// @tparam T The type of integer to generate.
/// @param min The minimum value to generate.
/// @param max The maximum value to generate.
/// @return A value in the range [min, max)
template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
T range(T min, T max) {
  gen::guard(min < max);
  auto _ = gen::group();

  auto out = min;

  gen::group([&] {
    while (out + 1 < max) {
      auto mid = lib::midpoint(out, max);
      if (gen::next(mid - out, max - mid))
        out = mid;
      else
        max = mid;
    }
  });

  return gen::shrink.to(min, out);
}

/// @brief Generates a random floating-point value in a range.
/// @tparam T The type of floating-point value to generate.
/// @param min The minimum value to generate.
/// @param max The maximum value to generate.
/// @return A value in the range [min, max)
template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T range(T min, T max) {
  gen::guard(min < max);

  auto out = min;

  gen::group([&] {
    for (std::size_t i = 0; i < sizeof(T) * CHAR_BIT; i++) {
      if (gen::next())
        out = lib::midpoint(out, max);
      else
        max = lib::midpoint(out, max);
    }
  });

  return gen::shrink.to(min, out);
}

/// @brief Generates a value in a range.
/// @tparam T The type of the minimum value to generate.
/// @tparam U The type of the maximum value to generate.
/// @param min The minimum value to generate.
/// @param max The maximum value to generate.
/// @return A value in the range [min, max).
template<int = 0, typename T, typename U, HALCHECK_REQUIRE(!std::is_same<T, U>())>
lib::common_type_t<T, U> range(T min, U max) {
  return gen::range<lib::common_type_t<T, U>>(min, max);
}

/// @brief Generates a random iterator in a range.
/// @tparam T The type of iterator to generate.
/// @param min The minimum iterator to generate.
/// @param max The maximum iterator to generate.
/// @return A value in the range [min, max).
template<typename T, HALCHECK_REQUIRE(lib::is_iterator<T>())>
T range(T min, T max) {
  return std::next(min, gen::range(0, std::distance(min, max)));
}

}} // namespace halcheck::gen

#endif
