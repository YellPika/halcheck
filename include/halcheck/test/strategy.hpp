#ifndef HALCHECK_TEST_STRATEGY_HPP
#define HALCHECK_TEST_STRATEGY_HPP

#include <halcheck/lib/any.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>

namespace halcheck { namespace test {

namespace concepts {

class strategy : public lib::concept_base {
public:
  template<typename T, HALCHECK_REQUIRE(lib::is_invocable<T, std::function<void()>>())>
  explicit strategy(lib::in_place_type_t<T>)
      : _invoke([](strategy &value, const std::function<void()> &func) { lib::invoke(value.as<T>(), func); }) {}

  void operator()(const std::function<void()> &func) { _invoke(*this, func); }

private:
  void (*_invoke)(strategy &, const std::function<void()> &);
};

} // namespace concepts

struct strategy : public lib::unique_poly<test::concepts::strategy> {
  using lib::unique_poly<test::concepts::strategy>::unique_poly;
};

test::strategy operator|(test::strategy, test::strategy);
test::strategy operator&(test::strategy, test::strategy);

}} // namespace halcheck::test

#endif
