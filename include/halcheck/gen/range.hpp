#ifndef HALCHECK_GEN_RANGE_HPP
#define HALCHECK_GEN_RANGE_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <iterator>
#include <limits>
#include <type_traits>

namespace halcheck { namespace gen {

static const struct range_t {
  /// @brief Generates a random integer in a range.
  /// @tparam T The type of integer to generate.
  /// @param min The minimum value to generate.
  /// @param max The maximum value to generate.
  /// @return A value in the range [min, max)
  template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
  T operator()(lib::atom id, T min, T max) const;

  /// @brief Generates a random floating-point value in a range.
  /// @tparam T The type of floating-point value to generate.
  /// @param min The minimum value to generate.
  /// @param max The maximum value to generate.
  /// @return A value in the range [min, max)
  template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
  T operator()(lib::atom id, T min, T max) const;

  /// @brief Generates a value in a range.
  /// @tparam T The type of the minimum value to generate.
  /// @tparam U The type of the maximum value to generate.
  /// @param min The minimum value to generate.
  /// @param max The maximum value to generate.
  /// @return A value in the range [min, max).
  template<typename T, typename U, HALCHECK_REQUIRE(!std::is_same<T, U>())>
  lib::common_type_t<T, U> operator()(lib::atom id, T min, U max) const;

  /// @brief Generates a random iterator in a range.
  /// @tparam T The type of iterator to generate.
  /// @param min The minimum iterator to generate.
  /// @param max The maximum iterator to generate.
  /// @return A value in the range [min, max).
  template<typename T, HALCHECK_REQUIRE(lib::is_forward_iterator<T>())>
  T operator()(lib::atom id, T min, T max) const;
} range;

template<typename T, HALCHECK_REQUIRE_(std::is_integral<T>())>
T range_t::operator()(lib::atom id, T min, T max) const {
  gen::guard(min < max);
  return gen::shrink_to(id, min, gen::sample(id, std::uintmax_t(max) - std::uintmax_t(min) - 1) + min);
}

template<typename T, HALCHECK_REQUIRE_(std::is_floating_point<T>())>
T range_t::operator()(lib::atom id, T min, T max) const {
  gen::guard(min < max);
  auto factor = T(gen::sample(id, std::numeric_limits<std::uintmax_t>::max() - 1)) /
                T(std::numeric_limits<std::uintmax_t>::max());
  return gen::shrink_to(id, min, min * factor + max * (1 - factor));
}

template<typename T, typename U, HALCHECK_REQUIRE_(!std::is_same<T, U>())>
lib::common_type_t<T, U> range_t::operator()(lib::atom id, T min, U max) const {
  using V = lib::common_type_t<T, U>;
  return (*this)(id, V(min), V(max));
}

template<typename T, HALCHECK_REQUIRE_(lib::is_forward_iterator<T>())>
T range_t::operator()(lib::atom id, T min, T max) const {
  return std::next(min, (*this)(id, 0, std::distance(min, max)));
}

extern template unsigned char range_t::operator()(lib::atom, unsigned char, unsigned char) const;
extern template signed char range_t::operator()(lib::atom, signed char, signed char) const;
extern template char range_t::operator()(lib::atom, char, char) const;
extern template unsigned short range_t::operator()(lib::atom, unsigned short, unsigned short) const;
extern template signed short range_t::operator()(lib::atom, signed short, signed short) const;
extern template unsigned int range_t::operator()(lib::atom, unsigned int, unsigned int) const;
extern template signed int range_t::operator()(lib::atom, signed int, signed int) const;
extern template unsigned long range_t::operator()(lib::atom, unsigned long, unsigned long) const;
extern template signed long range_t::operator()(lib::atom, signed long, signed long) const;
extern template unsigned long long range_t::operator()(lib::atom, unsigned long long, unsigned long long) const;
extern template signed long long range_t::operator()(lib::atom, signed long long, signed long long) const;
extern template float range_t::operator()(lib::atom, float, float) const;
extern template double range_t::operator()(lib::atom, double, double) const;
extern template long double range_t::operator()(lib::atom, long double, double) const;

}} // namespace halcheck::gen

#endif
