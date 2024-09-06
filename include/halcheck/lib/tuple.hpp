#ifndef HALCHECK_LIB_TUPLE_HPP
#define HALCHECK_LIB_TUPLE_HPP

namespace halcheck { namespace lib {

/// @brief A version of std::ignore usable with initializer lists.
struct ignore {
  template<typename... Args>
  constexpr explicit ignore(Args &&...) {}
};

}} // namespace halcheck::lib

#endif
