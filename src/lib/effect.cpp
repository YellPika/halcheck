#include "halcheck/lib/effect.hpp"

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/utility.hpp>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <vector>

using namespace halcheck;

std::size_t lib::effect::next() {
  static std::atomic_size_t value;
  return value++;
}

lib::move_only_function<lib::finally_t<>(bool)> lib::effect::clone_effect::fallback() const {
  return [](bool) {
    auto old = lib::exchange(current, &empty);
    return lib::finally([=] { current = old; });
  };
}

lib::effect::state::state(lib::in_place_t) : _impl(invoke<clone_effect>()) { assert(_impl); }

lib::finally_t<> lib::effect::state::handle() & {
  if (_impl)
    return _impl(false);

  auto old = lib::exchange(current, &empty);
  return lib::finally([=] { current = old; });
}

lib::finally_t<> lib::effect::state::handle() && {
  if (_impl)
    return _impl(true);

  auto old = lib::exchange(current, &empty);
  return lib::finally([=] { current = old; });
}

const lib::effect::context lib::effect::empty;
thread_local const lib::effect::context *lib::effect::current = &lib::effect::empty;
