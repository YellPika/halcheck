#include "halcheck/test/sized.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <functional>
#include <string>

using namespace halcheck;

namespace {
struct handler : eff::handler<handler, gen::size_effect> {
  explicit handler(std::uintmax_t size) : size(size) {}
  std::uintmax_t operator()(gen::size_effect) override { return size; }
  std::uintmax_t size;
};

struct strategy {
  explicit strategy(std::uintmax_t max_size) : max_size(max_size) {}

  void operator()(const std::function<void()> &func) {
    if (auto size_str = test::read("size")) {
      if (auto size_int = lib::of_string<std::uintmax_t>(*size_str))
        return eff::handle(func, handler(*size_int));
    }

    bool success = false;
    auto _ = lib::finally([&] {
      if (success) {
        ++size;
        if (max_size != 0)
          size %= max_size;
      }
    });

    test::write("size", std::to_string(size));

    try {
      eff::handle(func, handler(size));
      success = true;
    } catch (const gen::result &) {
      success = true;
      throw;
    }
  }

  std::uintmax_t max_size;
  std::uintmax_t size = 0;
};
} // namespace

test::strategy test::sized(std::uintmax_t max_size) { return test::make_strategy<::strategy>(max_size); }
