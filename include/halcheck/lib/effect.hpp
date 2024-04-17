#ifndef HALCHECK_LIB_EFFECT_HPP
#define HALCHECK_LIB_EFFECT_HPP

#include <halcheck/lib/scope.hpp>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace halcheck { namespace lib {

class context {
  static thread_local std::vector<void *> state;
  static std::atomic_size_t next;

  template<typename R, typename... Args>
  friend class effect;
};

/// @brief A poor-man's implementation of effects.
/// @tparam R The return type of the function.
/// @tparam Args The list of argument types for the function.
template<typename R, typename... Args>
class effect {
private:
  struct base {
    virtual ~base() = default;
    virtual R operator()(Args... args) = 0;
  };

  template<typename F>
  class fallback final : public base {
  public:
    fallback(F func) : _func(std::move(func)) {}

    R operator()(Args... args) override {
      std::vector<void *> state;
      auto _ = finally([&] { std::swap(context::state, state); });
      std::swap(context::state, state);
      return lib::invoke(_func, args...);
    }

  private:
    F _func;
  };

  template<typename F>
  class derived final : public base {
  public:
    derived(std::uintmax_t id, F func)
        : _id(id), _state(context::state), _func(std::move(func)), _prev(static_cast<base *>(this)) {
      if (id >= context::state.size())
        context::state.insert(context::state.end(), _id - context::state.size() + 1, nullptr);
      std::swap(context::state[id], _prev);
    }

    ~derived() { context::state[_id] = _prev; }

    R operator()(Args... args) override {
      auto state = _state;
      auto _ = finally([&] { std::swap(context::state, state); });
      std::swap(context::state, state);
      return lib::invoke(_func, args...);
    }

  private:
    std::uintmax_t _id;
    std::vector<void *> _state;
    F _func;
    void *_prev;
  };

  std::uintmax_t _id;
  std::unique_ptr<base> _def;

public:
  using handler = std::unique_ptr<base>;

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable_r<R, F, Args...>())>
  explicit effect(F func) : _id(context::next++), _def(new fallback<F>(std::move(func))) {}

  /// @brief Installs a handler for this effect. While this handler is in scope,
  ///        all calls to this effect will invoke func. Any calls to other
  ///        effects during the execution of func will have the same behaviour
  ///        as if they were called at the point func was installed.
  /// @tparam F The type of handler to install.
  /// @param func The handler to install.
  /// @return A value that will remove the handler upon destruction.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable_r<R, F, Args...>())>
  handler handle(F func) const {
    return handler(new derived<F>(_id, std::move(func)));
  }

  template<
      typename T,
      HALCHECK_REQUIRE(!lib::is_invocable_r<R, T, Args...>()),
      HALCHECK_REQUIRE(std::is_convertible<T, R>())>
  handler handle(T value) const {
    struct only {
      R operator()(Args...) { return value; }
      R value;
    };

    return handle(only{std::move(value)});
  }

  /// @brief Invokes the last active handler.
  /// @param args The list of arguments to pass to the handler.
  /// @return The result of calling the handler.
  R operator()(Args... args) const {
    if (_id < context::state.size() && context::state[_id])
      return (*reinterpret_cast<base *>(context::state[_id]))(args...);
    else
      return (*_def)(args...);
  }
};

}} // namespace halcheck::lib

#endif
