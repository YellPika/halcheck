#include "halcheck/lib/string.hpp" // IWYU pragma: keep

#include <halcheck/lib/optional.hpp>

#include <cstdlib>

using namespace halcheck;

lib::optional<std::string> lib::getenv(const std::string &name) {
  auto var = std::getenv(name.c_str()); // NOLINT: getenv is thread safe if we never use setenv
  if (var)
    return var;
  else
    return {};
}
