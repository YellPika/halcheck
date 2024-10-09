#ifndef HALCHECK_LIB_BIT_HPP
#define HALCHECK_LIB_BIT_HPP

/// @defgroup bit Bit Manipulation
/// @ingroup lib

#include <halcheck/lib/type_traits.hpp>

#include <cstring>

namespace halcheck { namespace lib {

/// @brief See https://en.cppreference.com/w/cpp/numeric/bit_cast.
/// @ingroup bit
template<
    typename T,
    typename U,
    HALCHECK_REQUIRE(sizeof(T) == sizeof(U)),
    HALCHECK_REQUIRE(std::is_trivially_copyable<T>()),
    HALCHECK_REQUIRE(std::is_trivially_copyable<U>()),
    HALCHECK_REQUIRE(std::is_trivially_constructible<T>())>
T bit_cast(const U &src) noexcept {
  T dest;
  std::memcpy(&dest, &src, sizeof(U));
  return dest;
}

}} // namespace halcheck::lib

#endif
