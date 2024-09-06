#ifndef HALCHECK_GEN_LABEL_HPP
#define HALCHECK_GEN_LABEL_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <initializer_list>
#include <string>
#include <utility>

namespace halcheck { namespace gen {

struct label_effect {
  lib::atom value;
  lib::destructable fallback() const { return nullptr; }
};

static struct label_t {
  lib::destructable operator()(lib::atom value) const { return eff::invoke<label_effect>(value); }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(lib::atom value, F func, Args &&...args) const {
    auto _ = eff::invoke<label_effect>(value);
    return lib::invoke(func, std::forward<Args>(args)...);
  }

  template<
      typename... Args,
      HALCHECK_REQUIRE(lib::is_invocable<label_t, Args...>()),
      HALCHECK_REQUIRE(sizeof...(Args) > 0)>
  lib::invoke_result_t<label_t, Args...> operator()(lib::atom value, Args &&...args) const {
    return (*this)(value, (*this), std::forward<Args>(args)...);
  }
} label;

template<typename F>
struct labelable {
  template<typename... Args, HALCHECK_REQUIRE(lib::is_invocable<const F &, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(lib::atom label, Args &&...args) const {
    return gen::label(label, static_cast<const F &>(*this), std::forward<Args>(args)...);
  }
};

}} // namespace halcheck::gen

#endif
