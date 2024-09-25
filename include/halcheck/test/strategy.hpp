#ifndef HALCHECK_TEST_STRATEGY_HPP
#define HALCHECK_TEST_STRATEGY_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>

namespace halcheck { namespace test {

class strategy {
public:
  template<
      typename F,
      typename... Args,
      HALCHECK_REQUIRE(std::is_constructible<F, Args...>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::function_view<void()>>())>
  explicit strategy(lib::in_place_type_t<F>, Args &&...args) : _impl(new derived<F>(std::forward<Args>(args)...)) {}

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<const lib::decay_t<F> &, lib::function_view<void()>>())>
  strategy(F &&func) // NOLINT: implicit conversion from functor
      : strategy(lib::in_place_type_t<lib::decay_t<F>>(), std::forward<F>(func)) {}

  void operator()(lib::function_view<void()> func) const { _impl->invoke(func); }

  bool has_value() const noexcept { return bool(_impl); }

  explicit operator bool() const noexcept { return bool(_impl); }

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

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(std::is_constructible<T, Args...>()),
    HALCHECK_REQUIRE(lib::is_invocable<const T &, lib::function_view<void()>>())>
test::strategy make_strategy(Args &&...args) {
  return test::strategy(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

test::strategy operator|(test::strategy, test::strategy);
test::strategy operator&(test::strategy, test::strategy);

}} // namespace halcheck::test

#endif
