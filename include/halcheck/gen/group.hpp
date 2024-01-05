#ifndef HALCHECK_GEN_GROUP_HPP
#define HALCHECK_GEN_GROUP_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>

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

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  lib::invoke_result_t<F> operator()(F func) const {
    auto _ = (*this)();
    return lib::invoke(func);
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

}} // namespace halcheck::gen

#endif
