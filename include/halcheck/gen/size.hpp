#ifndef HALCHECK_GEN_SIZE_HPP
#define HALCHECK_GEN_SIZE_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>

#include <cstdint>

namespace halcheck { namespace gen {

struct size_effect {
  std::uintmax_t fallback() const { return 0; }
};

inline std::uintmax_t size() { return lib::effect::invoke<size_effect>(); }

static const struct {
  struct handler : lib::effect::handler<handler, gen::size_effect> {
    explicit handler(double amount) : amount(amount) {}
    std::uintmax_t operator()(size_effect) final { return std::uintmax_t(double(gen::size()) * amount); }
    double amount;
  };

  handler::owning_scope operator()(double amount) const { return handler(amount).handle(); }

  template<typename F, typename... Args>
  lib::invoke_result_t<F, Args...> operator()(double amount, F func, Args &&...args) const {
    return handler(amount).handle(std::move(func), std::forward<Args>(args)...);
  }
} scale;

}} // namespace halcheck::gen

#endif
