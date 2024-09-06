#include "halcheck/lib/typeinfo.hpp"

#include <halcheck/lib/scope.hpp>

#include <atomic>

#ifdef HALCHECK_RTTI
#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <string>
#endif
#endif

using namespace halcheck;

#ifdef HALCHECK_RTTI
#ifdef __GNUG__
std::string lib::nameof(const std::type_info &info) {
  int status = 0;
  auto result = abi::__cxa_demangle(info.name(), nullptr, nullptr, &status);
  auto destroy = lib::finally([&] {
    std::free(result); // NOLINT: free is required here
  });
  return status == 0 ? result : info.name();
}
#else
std::string lib::nameof(const std::type_info &info) { return info.name(); }
#endif
#endif

std::size_t lib::type_id::next() {
  static std::atomic_size_t value{0};
  return ++value;
}
