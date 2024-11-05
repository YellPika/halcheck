#ifndef HALCHECK_LIB_NUMERIC_HPP
#define HALCHECK_LIB_NUMERIC_HPP

/**
 * @defgroup lib-numeric lib/numeric
 * @brief Utilities for numeric operations.
 * @see https://en.cppreference.com/w/cpp/header/numeric
 * @ingroup lib
 */

#include <halcheck/lib/type_traits.hpp>

#include <cmath>
#include <limits>

namespace halcheck { namespace lib {

/**
 * @brief An implementation of std::midpoint.
 * @see std::midpoint
 * @ingroup lib-numeric
 */
template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
constexpr T midpoint(T min, T max) noexcept {
  using U = lib::make_unsigned_t<T>;
  return min <= max ? min + T(U(max - min) >> 1) : min - T(U(min - max) >> 1);
}

/**
 * @brief An implementation of std::midpoint.
 * @see std::midpoint
 * @ingroup lib-numeric
 */
template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T midpoint(T min, T max) noexcept {
  static const T lo = std::numeric_limits<T>::min() * 2;
  static const T hi = std::numeric_limits<T>::max() / 2;
  auto amin = std::fabs(min);
  auto amax = std::fabs(max);
  return amin < hi && amax < hi ? (min + max) / 2
         : amin < lo            ? min + max / 2
         : amax < lo            ? min / 2 + max
                                : min / 2 + max / 2;
}

/**
 * @brief Converts an integral value into its equivalent unsigned version.
 * @tparam T The integral type to convert from.
 * @param value The value to convert.
 * @return If @p T is an unsigned type, then returns @p value. Otherwise, returns the result of casting @p value to a
 * signed type.
 * @ingroup lib-numeric
 */
template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
lib::make_unsigned_t<T> to_unsigned(T value) {
  return lib::make_unsigned_t<T>(value);
}

}} // namespace halcheck::lib

#endif
