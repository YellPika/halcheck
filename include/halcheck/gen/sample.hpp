#ifndef HALCHECK_GEN_SAMPLE_HPP
#define HALCHECK_GEN_SAMPLE_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace halcheck { namespace gen {

struct sample_effect {
  std::uintmax_t max;
  std::uintmax_t fallback() const { throw std::runtime_error("sample not handled"); }
};

static struct sample_t : gen::labelable<sample_t> {
  using gen::labelable<sample_t>::operator();

  std::uintmax_t operator()(std::uintmax_t max = std::numeric_limits<std::uintmax_t>::max()) const {
    return eff::invoke<sample_effect>(max);
  }

  bool operator()(std::uintmax_t w0, std::uintmax_t w1) const { return eff::invoke<sample_effect>(w0 + w1 - 1) >= w0; }
} sample;

}} // namespace halcheck::gen

#endif
