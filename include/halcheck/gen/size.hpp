#ifndef HALCHECK_GEN_SIZE_HPP
#define HALCHECK_GEN_SIZE_HPP

#include <halcheck/eff/api.hpp>

#include <cstdint>

namespace halcheck { namespace gen {

struct size_effect {
  std::uintmax_t fallback() const { return 0; }
};

inline std::uintmax_t size() { return eff::invoke<size_effect>(); }

}} // namespace halcheck::gen

#endif
