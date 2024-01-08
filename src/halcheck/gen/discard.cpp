#include "halcheck/gen/discard.hpp"

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/raise.hpp>
#include <halcheck/lib/type_traits.hpp>

using namespace halcheck;

lib::effect<lib::raise> gen::discard([] {
  std::terminate();
  return 0;
});

lib::effect<void> gen::succeed(std::terminate);

void gen::guard(bool cond) {
  if (!cond)
    gen::discard();
}
