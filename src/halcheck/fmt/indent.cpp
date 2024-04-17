#include "halcheck/fmt/indent.hpp"

#include <string>

using namespace halcheck;

fmt::indent::indent(std::ostream &os, std::size_t offset)
    : indent(os, [=](std::ostream &os) { os << std::string(offset, ' '); }) {}
