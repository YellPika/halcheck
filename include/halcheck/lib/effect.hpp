#ifndef HALCHECK_LIB_EFFECT_HPP
#define HALCHECK_LIB_EFFECT_HPP

#include <halcheck/lib/scope.hpp>

#include "halcheck/lib/type_traits.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

class context {
  static constexpr std::size_t size = 300;

  struct frame {
    std::uintmax_t id;
    void *handler;
  };

  static thread_local std::array<frame, size> stack;
  static thread_local std::size_t top;
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
  class derived final : public base {
  public:
    derived(std::uintmax_t id, F func)
        : _frame{id, static_cast<base *>(this)}, _top(context::top), _func(std::move(func)) {
      std::swap(context::stack[context::top++], _frame);
    }

    ~derived() override { std::swap(context::stack[--context::top], _frame); }

    R operator()(Args... args) override {
      context::top = _top;
      return lib::invoke(_func, args...);
    }

  private:
    context::frame _frame;
    std::size_t _top;
    F _func;
  };

  std::uintmax_t _id;
  std::unique_ptr<base> _def;

public:
  using handler = std::unique_ptr<base>;

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable_r<R, F, Args...>())>
  explicit effect(F func) : _id(context::next++), _def(new derived<F>(_id, std::move(func))) {}

  /// @brief Installs a handler for ths effect. While this handler is in scope,
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
    auto old = context::top;
    while (context::stack[context::top - 1].id != _id)
      --context::top;
    auto reset = lib::finally([&] { context::top = old; });
    return (*reinterpret_cast<base *>(context::stack[context::top - 1].handler))(std::forward<Args>(args)...);
  }
};

}} // namespace halcheck::lib

#endif
