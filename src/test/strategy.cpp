#include "halcheck/test/strategy.hpp"

#include <functional>
#include <utility>

using namespace halcheck;

namespace {
struct or_strategy {
  void operator()(std::function<void()> func) {
    lhs([&] { rhs(func); });
  }

  test::strategy lhs;
  test::strategy rhs;
};

struct and_strategy {
  void operator()(const std::function<void()> &func) {
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
