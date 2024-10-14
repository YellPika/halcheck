#ifndef HALCHECK_LIB_MEMORY_HPP
#define HALCHECK_LIB_MEMORY_HPP

/// @file
/// @brief Utilities for dynamic memory management.
/// @see https://en.cppreference.com/w/cpp/header/memory

#include <halcheck/lib/type_traits.hpp>

#include <memory> // IWYU pragma: export
#include <type_traits>

namespace halcheck { namespace lib {

/// @brief An implementation of std::make_unique.
/// @see std::make_unique
template<typename T, typename... Args, HALCHECK_REQUIRE(!std::is_array<T>())>
std::unique_ptr<T> make_unique(Args... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}} // namespace halcheck::lib

#endif
