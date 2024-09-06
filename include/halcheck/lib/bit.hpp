#ifndef HALCHECK_LIB_BIT_HPP
#define HALCHECK_LIB_BIT_HPP

#include <halcheck/lib/type_traits.hpp>

#include <cstring>

#if __cplusplus >= 201806L
#include <bit> // IWYU pragma: export
#endif

namespace halcheck { namespace lib {

#if __cplusplus >= 201806L
using std::bit_cast;
#else
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
#endif

}} // namespace halcheck::lib

#endif
