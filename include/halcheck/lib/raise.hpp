#ifndef HALCHECK_LIB_RAISE_HPP
#define HALCHECK_LIB_RAISE_HPP

/**
 * @defgroup lib-raise lib/raise
 * @brief A type for values that do not exist.
 * @ingroup lib
 */

#include <stdexcept>

namespace halcheck { namespace lib {

/**
 * @brief A type for values that do not exist.
 * @ingroup lib-raise
 */
struct raise {
  /**
   * @brief Throws the given value.
   * @tparam T The type of value to throw.
   * @param value The value to throw.
   */
  template<typename T>
  explicit raise(const T &value) {
    throw value;
  }

  /**
   * @brief A value of type @ref raise can be converted to any type.
   * @details Since @ref raise cannot be constructed, this function will never actually be run.
   * @tparam T The type of value to convert to.
   */
  template<typename T>
  [[noreturn]] operator T &() const { // NOLINT: raise can be converted to anything
    throw std::runtime_error("halcheck::lib::raise: impossible");
  }
};

}} // namespace halcheck::lib

#endif
