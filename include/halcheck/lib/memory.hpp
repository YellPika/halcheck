#ifndef HALCHECK_LIB_MEMORY_HPP
#define HALCHECK_LIB_MEMORY_HPP

#include <halcheck/lib/type_traits.hpp>

#include <memory> // IWYU pragma: export
#include <type_traits>

namespace halcheck { namespace lib {

template<typename T, typename... Args, HALCHECK_REQUIRE(!std::is_array<T>())>
std::unique_ptr<T> make_unique(Args... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T, HALCHECK_REQUIRE(!std::is_function<T>())>
constexpr T *to_address(T *p) noexcept {
  return p;
}

template<class T>
constexpr auto to_address(const T &p) noexcept -> decltype(lib::to_address(p.operator->())) {
  return lib::to_address(p.operator->());
}

}} // namespace halcheck::lib

#endif
