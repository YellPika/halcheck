#include "halcheck/test/strategy.hpp"

#include <functional>
#include <utility>

using namespace halcheck;

test::strategy test::operator|(test::strategy lhs, test::strategy rhs) {
  struct impl {
    void operator()(std::function<void()> func) {
      lhs([&] { rhs(func); });
    }

    test::strategy lhs;
    test::strategy rhs;
  };

  return impl{std::move(lhs), std::move(rhs)};
}

test::strategy test::operator&(test::strategy lhs, test::strategy rhs) {
  struct impl {
    void operator()(const std::function<void()> &func) {
      lhs(func);
      rhs(func);
    }

    test::strategy lhs;
    test::strategy rhs;
  };

  return impl{std::move(lhs), std::move(rhs)};
}
