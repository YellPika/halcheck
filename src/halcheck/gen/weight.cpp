#include "halcheck/gen/weight.hpp"

#include <utility>

using namespace halcheck::gen;

struct weight::tree {
  virtual ~tree() = default;
  virtual std::uintmax_t invoke(std::uintmax_t) const = 0;
};

namespace {

struct lit final : weight::tree {
  lit(std::uintmax_t value) : value(value) {}
  std::uintmax_t invoke(std::uintmax_t) const override { return value; }
  std::uintmax_t value;
};

struct var final : weight::tree {
  std::uintmax_t invoke(std::uintmax_t size) const override { return size; }
};

struct add final : weight::tree {
  add(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override { return lhs(size) + rhs(size); }
  weight lhs, rhs;
};

struct sub final : weight::tree {
  sub(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override {
    auto l = lhs(size);
    auto r = rhs(size);
    return l > r ? l - r : 0;
  }
  weight lhs, rhs;
};

struct mul final : weight::tree {
  mul(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override { return lhs(size) * rhs(size); }
  weight lhs, rhs;
};

struct quot final : weight::tree {
  quot(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override { return lhs(size) / rhs(size); }
  weight lhs, rhs;
};

struct mod final : weight::tree {
  mod(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override { return lhs(size) % rhs(size); }
  weight lhs, rhs;
};

struct comp final : weight::tree {
  comp(weight lhs, weight rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  std::uintmax_t invoke(std::uintmax_t size) const override { return lhs(rhs(size)); }
  weight lhs, rhs;
};

} // namespace

const weight weight::current = std::make_shared<var>();

weight::weight(std::uintmax_t value) : weight(std::make_shared<lit>(value)) {}

weight weight::compose(weight lhs, weight rhs) { return std::make_shared<comp>(std::move(lhs), std::move(rhs)); }

std::uintmax_t weight::operator()(std::uintmax_t size) const { return impl->invoke(size); }

weight &weight::operator+=(weight other) {
  *this = *this + std::move(other);
  return *this;
}

weight &weight::operator-=(weight other) {
  *this = *this - std::move(other);
  return *this;
}

weight &weight::operator*=(weight other) {
  *this = *this * std::move(other);
  return *this;
}

weight &weight::operator/=(weight other) {
  *this = *this / std::move(other);
  return *this;
}

weight &weight::operator%=(weight other) {
  *this = *this % std::move(other);
  return *this;
}

weight &weight::operator++() { return *this += 1; }

weight weight::operator++(int) {
  auto copy = *this;
  *this += 1;
  return copy;
}

weight &weight::operator--() { return *this -= 1; }

weight weight::operator--(int) {
  auto copy = *this;
  *this -= 1;
  return copy;
}

namespace halcheck { namespace gen {
weight operator+(weight lhs, weight rhs) { return std::make_shared<add>(std::move(lhs), std::move(rhs)); }
weight operator-(weight lhs, weight rhs) { return std::make_shared<sub>(std::move(lhs), std::move(rhs)); }
weight operator*(weight lhs, weight rhs) { return std::make_shared<mul>(std::move(lhs), std::move(rhs)); }
weight operator/(weight lhs, weight rhs) { return std::make_shared<quot>(std::move(lhs), std::move(rhs)); }
weight operator%(weight lhs, weight rhs) { return std::make_shared<mod>(std::move(lhs), std::move(rhs)); }
}} // namespace halcheck::gen
