#include "halcheck/gen/discard.hpp"

using namespace halcheck;

void gen::guard(bool cond) {
  if (!cond)
    throw gen::discard();
}
