#ifndef HALCHECK_LIB_TUPLE_HPP
#define HALCHECK_LIB_TUPLE_HPP

/// @file
/// @brief Utilities for std::tuple.
/// @see https://en.cppreference.com/w/cpp/header/tuple

#include <initializer_list>
#include <tuple> // IWYU pragma: export

namespace halcheck { namespace lib {

/// @brief The type of lib::ignore.
/// @see std::ignore
struct ignore_t {
  template<typename T>
  const ignore_t &operator=(T &&) const noexcept {
    return *this;
  }

  template<typename T>
  const ignore_t &operator=(const std::initializer_list<T> &) const noexcept {
    return *this;
  }
};

/// @brief A version of std::ignore usable with initializer lists.
/// @see std::ignore
static const lib::ignore_t ignore;

}} // namespace halcheck::lib

#endif
