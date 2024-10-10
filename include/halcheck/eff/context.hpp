#ifndef HALCHECK_EFF_CONTEXT_HPP
#define HALCHECK_EFF_CONTEXT_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <array>
#include <cstddef>
#include <functional>

namespace halcheck { namespace eff {

template<typename T>
using result_t = decltype(std::declval<T &>().fallback());

class context {
private:
  static std::size_t next();

  template<typename T>
  static std::size_t index() {
    static std::size_t value = next();
    return value;
  }

  static const std::size_t size = 64;

  struct entry {
    void *func;
    const std::array<entry, size> *state;
  };

  static thread_local const std::array<entry, size> empty, *current;

  struct clone_effect {
    std::function<lib::finally_t<>()> fallback() const;
  };

  template<typename T>
  struct base {
    virtual ~base() = default;
    virtual eff::result_t<T> operator()(T) = 0;
  };

public:
  template<typename T, typename... Args>
  class handler : private base<Args>..., private base<clone_effect> {
  private:
    friend class context;

    std::function<lib::finally_t<>()> operator()(clone_effect args) override {
      return std::bind(
          [](const std::function<lib::finally_t<>()> &context, T copy) {
            auto first = context();
            auto second = handle(std::move(copy));
            return std::move(first) + std::move(second);
          },
          invoke(args),
          *static_cast<T *>(this));
    }

    std::array<entry, size> install() {
      static_assert(std::is_base_of<handler, T>(), "T must be derived from handler<T, ...>");
      auto output = *current;
      lib::ignore = {(output[index<Args>()] = {static_cast<base<Args> *>(this), current})...};
      output[index<clone_effect>()] = {static_cast<base<clone_effect> *>(this), current};
      return output;
    }
  };

  template<typename T>
  static eff::result_t<T> invoke(T value) {
    auto &entry = (*current)[index<T>()];
    auto _ = lib::tmp_exchange(current, entry.state);
    if (entry.func)
      return (*reinterpret_cast<base<T> *>(entry.func))(std::move(value));
    else
      return std::move(value).fallback();
  }

  template<typename T>
  static lib::finally_t<> handle(T handler) {
    struct frame {
      explicit frame(T handler)
          : handler(std::move(handler)), copy(this->handler.install()), applier(current, &this->copy) {}

      void operator()() const { applier(); }

      T handler;
      std::array<entry, size> copy;
      lib::exchange_finally_t<const std::array<entry, size> *> applier;
    };

    return lib::finally(lib::move_only_function<void() &&>(lib::in_place_type_t<frame>(), std::move(handler)));
  }

  template<typename F, typename T>
  static lib::invoke_result_t<F> handle(F func, T &&handler) {
    auto copy = handler.install();
    auto _ = lib::tmp_exchange(current, &copy);
    return lib::invoke(func);
  }

  static std::function<lib::finally_t<>()> clone() { return invoke(clone_effect()); }

  static lib::finally_t<lib::exchange_finally_t<const std::array<entry, size> *>> reset() {
    return lib::tmp_exchange(current, &empty);
  }
};

}} // namespace halcheck::eff

#endif
