#ifndef HALCHECK_GEN_RANGE_HPP
#define HALCHECK_GEN_RANGE_HPP

/**
 * @defgroup gen-range gen/range
 * @brief Generating values in an interval.
 * @ingroup gen
 */

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

struct range_t {
  template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
  T operator()(lib::atom id, T min, T max) const {
    gen::guard(min < max);
    return gen::shrink_to(id, min, gen::sample(id, std::uintmax_t(max) - std::uintmax_t(min) - 1) + min);
  }

  template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
  T operator()(lib::atom id, T min, T max) const {
    gen::guard(min < max);
    auto factor = T(gen::sample(id, std::numeric_limits<std::uintmax_t>::max() - 1)) /
                  T(std::numeric_limits<std::uintmax_t>::max());
    return gen::shrink_to(id, min, min * factor + max * (1 - factor));
  }

  template<typename T, typename U, HALCHECK_REQUIRE(!std::is_same<T, U>())>
  lib::common_type_t<T, U> operator()(lib::atom id, T min, U max) const {
    using V = lib::common_type_t<T, U>;
    return (*this)(id, V(min), V(max));
  }

  template<typename T, HALCHECK_REQUIRE(lib::is_forward_iterator<T>())>
  T operator()(lib::atom id, T min, T max) const {
    return std::next(min, (*this)(id, 0, std::distance(min, max)));
  }
};

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

/**
 * @brief Generates a random value in an closed-open interval.
 * @par Signature
 * @code
 * template<typename T>
 * T operator()(lib::atom id, T min, T max)                        // (1)
 *
 * template<typename U, typename V>
 * lib::common_type_t<U, V> operator()(lib::atom id, U min, V max) // (2)
 * @endcode
 * @details An invocation of this function is well-formed only if of the following is true:
 * - `std::is_integral<T>()` holds.
 * - `std::is_integral<lib::common_type_t<U, V>>()` holds.
 * - `std::is_floating_point<T>()` holds.
 * - `std::is_floating_point<lib::common_type_t<U, V>>()` holds.
 * - `lib::is_forward_iterator<T>()` holds.
 * - `lib::is_forward_iterator<lib::common_type_t<U, V>>()` holds.
 *
 * @tparam T The type of value to generate.
 * @tparam U The type of the minimum value to generate.
 * @tparam V The type of the maximum value to generate.
 * @param id A unique identifier for the generated value.
 * @param min The minimum value to generate, inclusive.
 * @param max The maximum value to generate, exclusive.
 * @return A value in the range [min, max).
 * @ingroup gen-range
 */
static const range_t range;

}} // namespace halcheck::gen

#endif
