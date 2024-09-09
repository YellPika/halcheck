#ifndef HALCHECK_EFF_CONTEXT_HPP
#define HALCHECK_EFF_CONTEXT_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>

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
    std::function<lib::destructable()> fallback() const;
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

    std::function<lib::destructable()> operator()(clone_effect args) override {
      return std::bind(
          [](const std::function<lib::destructable()> &context, T copy) {
            auto first = context();
            auto second = handle(std::move(copy));
            return std::make_pair(std::move(first), std::move(second));
          },
          invoke(args),
          *static_cast<T *>(this));
    }

    std::array<entry, size> install() {
      static_assert(std::is_base_of<handler, T>(), "T must be derived from handler<T, ...>");
      auto output = *current;
      lib::ignore{(output[index<Args>()] = {static_cast<base<Args> *>(this), current})...};
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
  static lib::destructable handle(T handler) {
    struct frame {
      explicit frame(T handler)
          : handler(std::move(handler)), copy(this->handler.install()),
            applier(lib::tmp_exchange(current, &this->copy)) {}
      frame(frame &&) = delete;
      frame(const frame &) = delete;
      frame &operator=(frame &&) = delete;
      frame &operator=(const frame &) = delete;
      ~frame() = default;

      T handler;
      std::array<entry, size> copy;
      lib::finally_t<lib::exchange_finally_t<const std::array<entry, size> *>> applier;
    };

    return lib::make_destructable<frame>(std::move(handler));
  }

  template<typename F, typename T>
  static lib::invoke_result_t<F> handle(F func, T &&handler) {
    auto copy = handler.install();
    auto _ = lib::tmp_exchange(current, &copy);
    return lib::invoke(func);
  }

  static std::function<lib::destructable()> clone() { return invoke(clone_effect()); }

  static lib::finally_t<lib::exchange_finally_t<const std::array<entry, size> *>> reset() {
    return lib::tmp_exchange(current, &empty);
  }
};

}} // namespace halcheck::eff

#endif
