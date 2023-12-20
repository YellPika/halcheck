#include "halcheck/fmt/infer.hpp"

#include <utility>

using namespace halcheck;

static int index() {
  static int output = std::ostream::xalloc();
  return output;
}

bool fmt::infer(std::ostream &os) { return !os.iword(index()); }

bool fmt::infer(std::ostream &os, bool value) {
  long lvalue = !value;
  std::swap(lvalue, os.iword(index()));
  return !lvalue;
}
