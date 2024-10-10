#include "halcheck/eff/context.hpp"

#include <halcheck/lib/scope.hpp>

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

std::function<lib::finally_t<>()> eff::context::clone_effect::fallback() const {
  return [] { return lib::tmp_exchange(current, &empty); };
}

std::size_t eff::context::next() {
  static std::atomic_size_t value;
  auto output = value++;
  if (output >= size)
    throw std::runtime_error("too many effects");
  return output;
}
