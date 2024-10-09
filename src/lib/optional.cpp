#include "halcheck/lib/optional.hpp" // IWYU pragma: keep

#include <memory>
#include <mutex>
#include <type_traits>

using namespace halcheck;

struct non_move_assignable {
  non_move_assignable() = default;
  non_move_assignable(non_move_assignable &&) = default;
  non_move_assignable &operator=(non_move_assignable &&) = delete;
  non_move_assignable(const non_move_assignable &) = delete;
  non_move_assignable &operator=(const non_move_assignable &) = delete;
  ~non_move_assignable() = default;
};

using opt1 = lib::optional<std::mutex>;
using opt2 = lib::optional<std::unique_ptr<int>>;
using opt3 = lib::optional<int>;
using opt4 = lib::optional<lib::optional<std::mutex>>;
using opt5 = lib::optional<lib::optional<std::unique_ptr<int>>>;
using opt6 = lib::optional<lib::optional<int>>;
using opt7 = lib::optional<non_move_assignable>;

static_assert(std::is_nothrow_default_constructible<opt1>(), "");
static_assert(std::is_nothrow_default_constructible<opt2>(), "");
static_assert(std::is_nothrow_default_constructible<opt3>(), "");
static_assert(std::is_nothrow_default_constructible<opt4>(), "");
static_assert(std::is_nothrow_default_constructible<opt5>(), "");
static_assert(std::is_nothrow_default_constructible<opt6>(), "");
static_assert(std::is_nothrow_default_constructible<opt7>(), "");

static_assert(!std::is_move_constructible<opt1>(), "");
static_assert(std::is_move_constructible<opt2>(), "");
static_assert(std::is_nothrow_move_constructible<opt2>(), "");
static_assert(std::is_nothrow_move_constructible<opt3>(), "");
static_assert(!std::is_move_constructible<opt4>(), "");
static_assert(std::is_nothrow_move_constructible<opt5>(), "");
static_assert(std::is_nothrow_move_constructible<opt6>(), "");
static_assert(std::is_nothrow_move_constructible<opt7>(), "");
static_assert(std::is_nothrow_move_assignable<opt7>(), "");

static_assert(!std::is_copy_constructible<opt1>(), "");
static_assert(!std::is_copy_constructible<opt2>(), "");
static_assert(std::is_nothrow_copy_constructible<opt3>(), "");
static_assert(!std::is_copy_constructible<opt4>(), "");
static_assert(!std::is_copy_constructible<opt5>(), "");
static_assert(std::is_copy_constructible<opt6>(), "");
