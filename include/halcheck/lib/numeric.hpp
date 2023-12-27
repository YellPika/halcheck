#ifndef HALCHECK_LIB_NUMERIC_HPP
#define HALCHECK_LIB_NUMERIC_HPP

#include <halcheck/lib/type_traits.hpp>

#include <cmath>
#include <limits>

namespace halcheck { namespace lib {

#if __cplusplus >= 201902L
using std::midpoint;
#else
template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
constexpr T midpoint(T min, T max) noexcept {
  using U = lib::make_unsigned_t<T>;
  return min <= max ? min + T(U(max - min) >> 1) : min - T(U(min - max) >> 1);
}

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
#endif

}} // namespace halcheck::lib

#endif
