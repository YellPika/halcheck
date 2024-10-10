#ifndef HALCHECK_GEN_LABEL_HPP
#define HALCHECK_GEN_LABEL_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <utility>

namespace halcheck { namespace gen {

struct label_effect {
  lib::atom value;
  lib::finally_t<> fallback() const { return lib::finally_t<>(); }
};

static struct {
  lib::finally_t<> operator()(lib::atom value) const { return eff::invoke<label_effect>(value); }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(lib::atom value, F func, Args &&...args) const {
    auto _ = eff::invoke<label_effect>(value);
    return lib::invoke(func, std::forward<Args>(args)...);
  }
} label;

}} // namespace halcheck::gen

#endif
