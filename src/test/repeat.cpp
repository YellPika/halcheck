#include "halcheck/test/repeat.hpp"

#include <halcheck/gen/discard.hpp>
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <functional>

using namespace halcheck;

test::strategy test::repeat(std::uintmax_t max_success, std::uintmax_t discard_ratio) {
  return [=](const std::function<void()> &func) {
    std::uintmax_t discarded = 0;
    std::uintmax_t successes = 0;
    while (max_success == 0 || successes < max_success) {
      try {
        func();
        ++successes;
      } catch (const gen::discard &) {
        if (discard_ratio > 0 && ++discarded / discard_ratio >= max_success)
          throw test::discard_limit_exception();
      } catch (const gen::succeed &) {
        break;
      }
    }
  };
}
