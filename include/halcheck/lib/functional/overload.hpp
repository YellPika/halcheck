#ifndef HALCHECK_LIB_FUNCTIONAL_OVERLOAD_HPP
#define HALCHECK_LIB_FUNCTIONAL_OVERLOAD_HPP

// IWYU pragma: private, include <halcheck/lib/functional.hpp>

#include <utility>

namespace halcheck { namespace lib {

/**
 * @brief Represents an overload set as a single value.
 * @tparam Args The remaining overloads to represent.
 * @ingroup lib-functional
 */
template<typename... Args>
struct overload {
#ifdef HALCHECK_DOXYGEN
  /**
   * @brief Construct a new overload set.*@param args The set of overloads to represent.
   * @post `overload(fs...)(xs...) = f(xs...)`, where @c f is the unique element of @c fs such that
   *       `lib::is_invocable<decltype(f), decltype(xs)...>()` is satisfied.
   */
  explicit overload(Args... args);
#endif
};

template<typename T>
struct overload<T> : private T {
  using T::operator();
  explicit overload(T head) : T(std::move(head)) {}
};

template<typename T, typename... Args>
struct overload<T, Args...> : private T, private overload<Args...> {
  using T::operator();
  using overload<Args...>::operator();
  explicit overload(T head, Args... tail) : T(std::move(head)), overload<Args...>(std::move(tail)...) {}
};

/**
 * @brief Constructs an overloaded functor from a set of pre-existing functors.
 * @tparam Args The types of functors to combine.
 * @param args The functors to combine.
 * @return `lib::overload<Args...>(std::move(args)...)`.
 * @ingroup lib-functional
 */
template<typename... Args>
lib::overload<Args...> make_overload(Args... args) {
  return lib::overload<Args...>(std::move(args)...);
}

}} // namespace halcheck::lib

#endif
