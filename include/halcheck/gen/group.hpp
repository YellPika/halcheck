#ifndef HALCHECK_GEN_GROUP_HPP
#define HALCHECK_GEN_GROUP_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

namespace halcheck { namespace gen {

extern const class group_t : private lib::effect<void, bool> {
private:
  struct ungroup {
    void operator()() const { self->effect::operator()(false); }
    const group_t *self;
  };

public:
  using effect::handler;
  using scope = lib::finally_t<ungroup>;
  using effect::effect;

  scope operator()() const {
    effect::operator()(true);
    return lib::finally(ungroup{this});
  }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(F func, Args &&...args) const {
    auto _ = (*this)();
    return lib::invoke(func, std::forward<Args>(args)...);
  }

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, bool>())>
  handler handle(F func) const {
    struct invoke {
      void operator()(bool open) {
        self->effect::operator()(open);
        lib::invoke(func, open);
      }
      const group_t *self;
      F func;
    };
    return effect::handle(invoke{this, std::move(func)});
  }
} group;

template<
    typename F1,
    typename F2 = void (*)(),
    HALCHECK_REQUIRE(lib::is_invocable<F1>()),
    HALCHECK_REQUIRE(lib::is_invocable<F2>())>
lib::common_type_t<lib::invoke_result_t<F1>, lib::invoke_result_t<F2>> if_(
    bool cond, F1 f1, F2 f2 = [] {}) {
  if (cond) {
    auto output = gen::group(f1);
    gen::group();
    return output;
  } else {
    gen::group();
    return gen::group(f2);
  }
}

template<typename F, HALCHECK_REQUIRE(lib::is_invocable_r<bool, F>())>
void while_(F func) {
  auto _ = gen::group();
  while (gen::group(func))
    ;
}

template<
    typename I,
    typename F,
    HALCHECK_REQUIRE(lib::is_iterator<I>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, typename std::iterator_traits<I>::reference>())>
void for_each(I begin, I end, F func) {
  auto _ = gen::group();
  for (auto i = begin; i != end; ++i)
    gen::group(func, *i);
}

template<
    typename R,
    typename F,
    HALCHECK_REQUIRE(lib::is_range<R>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, lib::range_reference_t<R>>())>
void for_each(R range, F func) {
  gen::for_each(lib::begin(range), lib::end(range), std::move(func));
}

}} // namespace halcheck::gen

#endif
