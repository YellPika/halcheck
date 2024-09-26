#ifndef HALCHECK_LIB_TUPLE_HPP
#define HALCHECK_LIB_TUPLE_HPP

#include <initializer_list>
#include <tuple> // IWYU pragma: export

namespace halcheck { namespace lib {

/// @brief A version of std::ignore usable with initializer lists.
static const struct ignore_t {
  template<typename T>
  const ignore_t &operator=(T &&) const noexcept {
    return *this;
  }

  template<typename T>
  const ignore_t &operator=(const std::initializer_list<T> &) const noexcept {
    return *this;
  }
} ignore;

}} // namespace halcheck::lib

#endif
