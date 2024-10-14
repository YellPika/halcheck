#ifndef HALCHECK_LIB_BIT_HPP
#define HALCHECK_LIB_BIT_HPP

/// @file
/// @brief Bit-level operations.
/// @see https://en.cppreference.com/w/cpp/header/bit

#include <halcheck/lib/type_traits.hpp>

#include <cstring>

namespace halcheck { namespace lib {

/// @brief Performs a bit-level conversion from one type to another.
/// @see std::bit_cast
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
