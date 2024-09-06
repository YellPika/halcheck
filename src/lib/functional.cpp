#include "halcheck/lib/functional.hpp"

#include <halcheck/lib/type_traits.hpp>

#include <memory>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

using namespace halcheck::lib;

static const auto lambda0 = [] {};
static const auto lambda1 = [](int x) { return x; };

static_assert(is_invocable<decltype(lambda0)>(), "");
static_assert(!is_invocable<decltype(lambda0), int>(), "");
static_assert(!is_invocable<decltype(lambda0), int>(), "");
static_assert(is_invocable<decltype(lambda1), int>(), "");

static_assert(is_invocable_r<void, decltype(lambda0)>(), "");
static_assert(!is_invocable_r<int, decltype(lambda0), int>(), "");
static_assert(!is_invocable_r<void, decltype(lambda0), int>(), "");
static_assert(is_invocable_r<int, decltype(lambda1), int>(), "");

static_assert(!is_movable<decltype(lambda0)>(), "");
static_assert(is_movable<assignable<decltype(lambda0)>>(), "");
static_assert(!is_copyable<decltype(lambda0)>(), "");
static_assert(is_copyable<assignable<decltype(lambda0)>>(), "");

namespace {
struct lambda2 {
  void operator()();
  std::unique_ptr<int> p;
};
} // namespace

static_assert(!is_copyable<assignable<lambda2>>(), "");
