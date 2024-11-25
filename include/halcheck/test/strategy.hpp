#ifndef HALCHECK_TEST_STRATEGY_HPP
#define HALCHECK_TEST_STRATEGY_HPP

/**
 * @defgroup test-strategy test/strategy
 * @brief Test strategies.
 * @ingroup test
 */

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>

namespace halcheck { namespace test {

/**
 * @brief A @ref strategy executes a nullary function while handling [effects](@ref lib-effect) or performing other
 * operations such as logging.
 */
class strategy {
public:
  /**
   * @brief Constructs an empty @ref strategy.
   * @post `strategy()(func) == func()`
   */
  strategy() = default;

  /**
   * @brief Constructs a @ref strategy.
   * @tparam F The type providing an implementation for `operator()`.
   * @tparam Args The types of arguments used to construct a value of type @p F.
   * @param args The arguments used to construct a value of type @p F.
   * @post `strategy(lib::in_place_type_t<F>(), args...)(func) == lib::invoke(F(args...), func)`
   */
  template<
      typename F,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<F, Args...>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::function_view<void()>>())>
  explicit strategy(lib::in_place_type_t<F>, Args &&...args) : _impl(new derived<F>(std::forward<Args>(args)...)) {}

  /**
   * @brief Constructs a @ref strategy.
   * @tparam F The type providing an implementation for `operator()`.
   * @param func The function to invoke for `operator()`.
   * @post `strategy(f)(func) == lib::invoke(f, func)`
   */
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<const lib::decay_t<F> &, lib::function_view<void()>>())>
  strategy(F &&func) // NOLINT: implicit conversion from functor
      : strategy(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(func)) {}

  /**
   * @brief Executes the function stored in this @ref strategy.
   * @param func The function to pass as an argument.
   */
  void operator()(lib::function_view<void()> func) const {
    if (_impl)
      _impl->invoke(func);
    else
      lib::invoke(func);
  }

private:
  struct base {
    virtual ~base() = default;
    virtual void invoke(lib::function_view<void()>) const = 0;
  };

  template<typename T>
  struct derived final : base {
    template<typename... Args>
    explicit derived(Args &&...args) : value(std::forward<Args>(args)...) {}

    void invoke(lib::function_view<void()> func) const override { lib::invoke(value, func); }

    T value;
  };

  std::unique_ptr<base> _impl;
};

/**
 * @brief Constructs a @ref strategy of the given type.
 * @tparam F The type of function to store in the @ref strategy.
 * @tparam Args The types of arguments used to construct a value of type @p F.
 * @param args The arguments used to construct a value of type @p F.
 * @return `test::strategy(lib::in_place_type_t<F>(), std::forward<Args>(args)...)`
 */
template<
    typename F,
    typename... Args,
    HALCHECK_REQUIRE(std::is_constructible<F, Args...>()),
    HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::function_view<void()>>())>
test::strategy make_strategy(Args &&...args) {
  return test::strategy(lib::in_place_type_t<F>(), std::forward<Args>(args)...);
}

/**
 * @brief Composes one strategy inside another.
 * @param lhs,rhs The strategies to compose.
 * @return The composed strategy.
 * @post `(f | g)(h) == f([&] { g(h); })`
 */
test::strategy operator|(test::strategy lhs, test::strategy rhs);

/**
 * @brief Composes two strategies sequentially.
 * @param lhs,rhs The strategies to compose.
 * @return The composed strategy.
 * @post `(f & g)(h) == (f(h); g(h))`
 */
test::strategy operator&(test::strategy lhs, test::strategy rhs);

}} // namespace halcheck::test

#endif
