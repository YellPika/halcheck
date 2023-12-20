#ifndef HALCHECK_LIB_TUPLE_HPP
#define HALCHECK_LIB_TUPLE_HPP

#include <initializer_list>

namespace halcheck { namespace lib {

/// @brief A version of std::ignore that can be assigned initializer lists.
static constexpr struct ignore_t {
  template<typename T>
  void operator=(T &&) const noexcept {}

  template<typename T>
  void operator=(const std::initializer_list<T> &) const noexcept {}
} ignore;

}} // namespace halcheck::lib

#endif
