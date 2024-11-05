#ifndef HALCHECK_LIB_FUNCTIONAL_CONSTRUCTOR_HPP
#define HALCHECK_LIB_FUNCTIONAL_CONSTRUCTOR_HPP

// IWYU pragma: private, include <halcheck/lib/functional.hpp>

#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace lib {

/**
 * @brief A function object that calls the constructor of a given type.
 * @tparam T The type of value to construct.
 * @ingroup lib-functional
 */
template<typename T>
struct constructor {
  /**
   * @brief Invokes a constructor of @p T.
   * @tparam Args The type of arguments to pass to @p T::T. Must satisfy
   * <tt> @ref "std::is_constructible"<T, Args...>() </tt>.
   * @param args The arguments to pass to @p T::T.
   * @return The constructed object.
   */
  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  T operator()(Args... args) {
    return T(std::forward<Args>(args)...);
  }
};

}} // namespace halcheck::lib

#endif
