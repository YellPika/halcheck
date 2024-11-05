#ifndef HALCHECK_LIB_FUNCTIONAL_FUNCTION_VIEW_HPP
#define HALCHECK_LIB_FUNCTIONAL_FUNCTION_VIEW_HPP

// IWYU pragma: private, include <halcheck/lib/functional.hpp>

#include <halcheck/lib/functional/invoke.hpp>
#include <halcheck/lib/functional/overload.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

namespace halcheck { namespace lib {

template<typename T>
class function_view;

/**
 * @brief Represents a reference to a function object.
 * @tparam R The return type of the function to refer to.
 * @tparam Args The argument types of the function to refer to.
 * @ingroup lib-functional
 */
template<typename R, typename... Args>
class function_view<R(Args...)> {
public:
  /**
   * @brief Constructs a new function view from a function object.
   * @tparam F The type of function to refer to.
   * @param func The function to refer to.
   * @post <tt> function_view(f)(xs...) == @ref "lib::invoke"(f, xs...) </tt>
   */
  template<
      typename F,
      HALCHECK_REQUIRE(!std::is_const<F>()),
      HALCHECK_REQUIRE(!std::is_pointer<lib::decay_t<F>>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<R, lib::decay_t<F> &, Args...>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<F>, function_view>())>
  function_view(F &&func) noexcept // NOLINT: implicit reference to functor
      : _impl(closure{std::addressof(func), [](void *impl, Args... args) {
                        return lib::invoke(*reinterpret_cast<lib::decay_t<F> *>(impl), std::forward<Args>(args)...);
                      }}) {}

  /**
   * @brief Constructs a new function view from a function pointer.
   * @param func The function to refer to.
   * @post <tt> function_view(f)(xs...) == f(xs...) </tt>
   */
  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  /**
   * @brief Calls the underlying function.
   * @param args The arguments to pass to the function.
   * @return The result of calling the underlying function with the given arguments.
   */
  R operator()(Args... args) const {
    return lib::visit(
        lib::make_overload(
            [&](const closure &func) { return func.invoke(func.self, std::forward<Args>(args)...); },
            [&](R (*func)(Args...)) { return func(std::forward<Args>(args)...); }),
        _impl);
  }

private:
  struct closure {
    void *self;
    R (*invoke)(void *, Args...);
  };

  lib::variant<R (*)(Args...), closure> _impl;
};

/**
 * @brief Represents a const reference to a function object.
 * @tparam R The return type of the function to refer to.
 * @tparam Args The argument types of the function to refer to.
 * @ingroup lib-functional
 */
template<typename R, typename... Args>
class function_view<R(Args...) const> {
public:
  /**
   * @brief Constructs a new function view from a function object.
   * @tparam F The type of function to refer to.
   * @param func The function to refer to.
   * @post function_view(f)(xs...) == lib::invoke(f, xs...)
   */
  template<
      typename F,
      HALCHECK_REQUIRE(!std::is_pointer<lib::decay_t<F>>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<R, const lib::decay_t<F> &, Args...>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<F>, function_view>())>
  function_view(F &&func) noexcept // NOLINT: implicit reference to functor
      : _impl(closure{
            std::addressof(func), [](const void *impl, Args... args) {
              return lib::invoke(*reinterpret_cast<const lib::decay_t<F> *>(impl), std::forward<Args>(args)...);
            }}) {}

  /**
   * @brief Constructs a new function view from a function pointer.
   * @param func The function to refer to.
   * @post function_view(f)(xs...) == f(xs...)
   */
  function_view(R (*func)(Args...)) noexcept // NOLINT: implicit copy of function pointer
      : _impl(func) {}

  /**
   * @brief Calls the underlying function.*@param args The arguments to pass to the function.
   * @return The result of calling the underlying function with the given arguments.
   */
  R operator()(Args... args) const {
    return lib::visit(
        lib::make_overload(
            [&](const closure &func) { return func.invoke(func.self, std::forward<Args>(args)...); },
            [&](R (*func)(Args...)) { return func(std::forward<Args>(args)...); }),
        _impl);
  }

private:
  struct closure {
    const void *self;
    R (*invoke)(const void *, Args...);
  };

  lib::variant<R (*)(Args...), closure> _impl;
};

}} // namespace halcheck::lib

#endif
