#ifndef HALCHECK_LIB_ITERATOR_MOVE_HPP
#define HALCHECK_LIB_ITERATOR_MOVE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/type_traits.hpp>

#include <utility>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/iterator/ranges/iter_move

inline namespace iter_move_cpo {
static const class {
private:
  template<typename T>
  using free = decltype(iter_move(std::declval<T &&>()));

public:
  template<typename T, HALCHECK_REQUIRE(lib::is_detected<free, T>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(iter_move(value))) -> decltype(iter_move(value)) {
    return iter_move(value);
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_detected<free, T>()),
      HALCHECK_REQUIRE(std::is_lvalue_reference<T &&>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(std::move(*value))) -> decltype(std::move(*value)) {
    return std::move(*value);
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_detected<free, T>()),
      HALCHECK_REQUIRE(!std::is_lvalue_reference<T &&>())>
  constexpr auto operator()(T &&value) const noexcept(noexcept(*value)) -> decltype(*value) {
    return *value;
  }
} iter_move;
} // namespace iter_move_cpo

}} // namespace halcheck::lib

#endif
