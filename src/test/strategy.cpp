#include "halcheck/test/strategy.hpp"

#include <halcheck/lib/functional.hpp>

#include <utility>

using namespace halcheck;

namespace {
struct or_strategy {
  void operator()(lib::function_view<void()> func) const {
    lhs([&] { rhs(func); });
  }

  test::strategy lhs;
  test::strategy rhs;
};

struct and_strategy {
  void operator()(lib::function_view<void()> func) const {
    lhs(func);
    rhs(func);
  }

  test::strategy lhs;
  test::strategy rhs;
};
} // namespace

test::strategy test::operator|(test::strategy lhs, test::strategy rhs) {
  return or_strategy{std::move(lhs), std::move(rhs)};
}

test::strategy test::operator&(test::strategy lhs, test::strategy rhs) {
  return and_strategy{std::move(lhs), std::move(rhs)};
}
