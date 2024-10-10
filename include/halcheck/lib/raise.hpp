#ifndef HALCHECK_LIB_RAISE_HPP
#define HALCHECK_LIB_RAISE_HPP

/// @file

#include <stdexcept>

namespace halcheck { namespace lib {

/// @brief A type for values that do not exist.
/// @ingroup utility
struct raise {
  /// @brief Throws the given value.
  /// @tparam T The type of value to throw.
  /// @param value The value to throw.
  template<typename T>
  explicit raise(const T &value) {
    throw value;
  }

  /// @brief Produces a value of arbitrary type. Since raise cannot be
  ///        constructed, this should never actually occur.
  template<typename T>
  [[noreturn]] operator T &() const { // NOLINT: raise can be converted to anything
    throw std::runtime_error("halcheck::lib::raise: impossible");
  }
};

}} // namespace halcheck::lib

#endif
