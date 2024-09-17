#ifndef HALCHECK_LIB_MEMORY_HPP
#define HALCHECK_LIB_MEMORY_HPP

#include <halcheck/lib/type_traits.hpp>

#include <memory> // IWYU pragma: export

namespace halcheck { namespace lib {

template<typename T, typename... Args, HALCHECK_REQUIRE(!std::is_array<T>())>
std::unique_ptr<T> make_unique(Args... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}} // namespace halcheck::lib

#endif
