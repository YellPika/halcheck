#include "halcheck/lib/typeinfo.hpp"

#include <halcheck/lib/scope.hpp>

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#endif

using namespace halcheck;

#ifdef __GNUG__
std::string lib::nameof(const std::type_info &info) {
  int status;
  auto result = abi::__cxa_demangle(info.name(), nullptr, nullptr, &status);
  auto destroy = lib::finally([&] { std::free(result); });
  return status == 0 ? result : info.name();
}
#else
std::string lib::nameof(const std::type_info &info) { return info.name(); }
#endif
