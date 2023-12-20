#ifndef HALCHECK_LIB_SCOPE_HPP
#define HALCHECK_LIB_SCOPE_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <functional>

namespace halcheck { namespace lib {

template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
class finally_t {
public:
  explicit finally_t(F func) noexcept : _func(std::move(func)) {}
  finally_t(finally_t &&other) noexcept : _func(std::move(other._func)) { other._invoke = false; }
  finally_t(const finally_t &) = delete;
  finally_t &operator=(const finally_t &) = delete;
  finally_t &operator=(finally_t &&) = delete;
  void *operator new(std::size_t) = delete;
  void *operator new[](std::size_t) = delete;

  ~finally_t() noexcept {
    if (_invoke)
      lib::invoke(std::move(_func));
  }

private:
  bool _invoke = true;
  F _func;
};

/// @brief Executes a function on scope exit.
/// @tparam F the type of function to execute.
/// @param func The function to execute.
/// @return An object that executes the function upon destruction.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
finally_t<F> finally(F func) {
  return finally_t<F>(std::move(func));
}

}} // namespace halcheck::lib

#endif
