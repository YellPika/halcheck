#ifndef HALCHECK_GEN_WEIGHT_HPP
#define HALCHECK_GEN_WEIGHT_HPP

#include <cstdint>
#include <memory>

namespace halcheck { namespace gen {

class weight {
public:
  static const weight current;

  weight(std::uintmax_t value = 0);

  std::uintmax_t operator()(std::uintmax_t size) const;

  static weight compose(weight, weight);

  weight &operator+=(weight);
  weight &operator-=(weight);
  weight &operator*=(weight);
  weight &operator/=(weight);
  weight &operator%=(weight);

  weight &operator++();
  weight operator++(int);
  weight &operator--();
  weight operator--(int);

  struct tree;

private:
  template<typename T>
  weight(std::shared_ptr<T> impl) : impl(std::move(impl)) {}

  std::shared_ptr<const tree> impl;

  friend weight operator+(weight, weight);
  friend weight operator-(weight, weight);
  friend weight operator*(weight, weight);
  friend weight operator/(weight, weight);
  friend weight operator%(weight, weight);
};

}} // namespace halcheck::gen

#endif
