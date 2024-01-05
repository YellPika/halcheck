#ifndef HALCHECK_GEN_NEXT_HPP
#define HALCHECK_GEN_NEXT_HPP

#include <halcheck/lib/effect.hpp>

#include <cstdint>

namespace halcheck { namespace gen {

class weight {
public:
  weight() = default;
  weight(std::uintmax_t value) : value(value) {}

  weight &operator+=(weight other) {
    value += other.value;
    return *this;
  }

  weight &operator-=(weight other) {
    value -= other.value;
    return *this;
  }

  weight &operator*=(weight other) {
    value *= other.value;
    return *this;
  }

  weight &operator/=(weight other) {
    value /= other.value;
    return *this;
  }

  weight &operator%=(weight other) {
    value %= other.value;
    return *this;
  }

  weight &operator--() {
    --value;
    return *this;
  }
  weight &operator++() {
    ++value;
    return *this;
  }

  weight operator--(int) { return value--; }
  weight operator++(int) { return value++; }

private:
  friend weight operator+(weight lhs, weight rhs) { return lhs.value + rhs.value; }
  friend weight operator-(weight lhs, weight rhs) { return lhs.value - rhs.value; }
  friend weight operator*(weight lhs, weight rhs) { return lhs.value * rhs.value; }
  friend weight operator/(weight lhs, weight rhs) { return lhs.value / rhs.value; }
  friend weight operator%(weight lhs, weight rhs) { return lhs.value % rhs.value; }

  std::uintmax_t value;

  friend class next_t;
};

extern const class next_t : private lib::effect<bool, std::uintmax_t, std::uintmax_t> {
public:
  using effect::effect;
  using effect::handle;

  /// @brief Obtains the next bit of randomness.
  /// @param w0 The relative weight for returning false.
  /// @param w1 The relative weight for returning true.
  /// @return An arbitrary boolean. Note that the resulting output distribution is
  ///         not guaranteed to correspond to the given weights. Furthermore,
  ///         passing a weight of zero does not guarantee that the corresponding
  ///         value will not be returned.
  bool operator()(const gen::weight &w0 = 1, const gen::weight &w1 = 1) const {
    return effect::operator()(w0.value, w1.value);
  }
} next;

extern const lib::effect<gen::weight> size;

}} // namespace halcheck::gen

#endif
