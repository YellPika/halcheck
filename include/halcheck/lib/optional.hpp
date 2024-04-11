#ifndef HALCHECK_LIB_OPTIONAL_HPP
#define HALCHECK_LIB_OPTIONAL_HPP

#if __cplusplus >= 201606L
#include <optional>
#define HALCHECK_OPTIONAL_NS ::std
#else
#include <tl/optional.hpp>
#define HALCHECK_OPTIONAL_NS ::tl
#endif

namespace halcheck { namespace lib {

using HALCHECK_OPTIONAL_NS::nullopt;
using HALCHECK_OPTIONAL_NS::nullopt_t;
using HALCHECK_OPTIONAL_NS::optional;

template<typename T>
struct constexpr_optional {
public:
  constexpr constexpr_optional() : has_value(false), value() {}
  constexpr constexpr_optional(T value) : has_value(true), value(value) {}

  operator lib::optional<T>() const {
    if (has_value)
      return value;
    else
      return {};
  }

  T value_or(T other) const { return has_value ? value : other; }

private:
  bool has_value;
  T value;
};

}} // namespace halcheck::lib

#endif
