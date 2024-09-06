#include "halcheck/eff/context.hpp"

#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/utility.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <stdexcept>

using namespace halcheck;

thread_local const std::array<eff::context::entry, eff::context::size> eff::context::empty = []() noexcept {
  std::array<entry, size> output{};
  for (std::size_t i = 0; i < size; i++)
    output[i] = {nullptr, &output};
  return output;
}();

thread_local const std::array<eff::context::entry, eff::context::size> *eff::context::current = &empty;

eff::context::apply::apply(const std::array<entry, size> &state) : state(lib::exchange(current, &state)) {}

eff::context::apply::~apply() { current = state; }

std::function<lib::destructable()> eff::context::clone_effect::fallback() const {
  return [] { return lib::make_destructable<apply>(empty); };
}

std::size_t eff::context::next() {
  static std::atomic_size_t value;
  auto output = value++;
  if (output >= size)
    throw std::runtime_error("too many effects");
  return output;
}
