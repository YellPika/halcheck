#ifndef HALCHECK_LIB_EFFECT_HPP
#define HALCHECK_LIB_EFFECT_HPP

/**
 * @defgroup lib-effect lib/effect
 * @brief Scoped algebraic effects (a.k.a. resumable exceptions.)
 * @ingroup lib
 */

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include <vector>

namespace halcheck { namespace lib {

#ifdef HALCHECK_DOXYGEN
/**
 * @brief Invokes the fallback behaviour for an effect.
 * @tparam T The type of effect to invoke.
 * @param args The arguments to the effect.
 * @ingroup lib-effect
 */
lib::effect_result_t<T> fallback(T args);
#endif

static const class {
private:
  template<typename T>
  using member_t = decltype(std::declval<const T &>().fallback());

  template<typename T>
  using free_t = decltype(fallback(std::declval<const T &>()));

public:
  template<typename T>
  member_t<T> operator()(T args) const {
    return std::move(args).fallback();
  }

  template<typename T, HALCHECK_REQUIRE(!lib::is_detected<member_t, T>())>
  free_t<T> operator()(T args) const {
    return fallback(std::move(args));
  }
} fallback;

/**
 * @brief Evaluates to the result type of the given effect.
 * @tparam T The type to query.
 * @ingroup lib-effect
 */
template<typename T>
using effect_result_t = lib::invoke_result_t<decltype(lib::fallback), const T &>;

/**
 * @brief Determines if a given type is an effect.
 * @tparam T The type to query.
 * @details The type @p T is an effect if and only if one of the following conditions hold:
 * - The expression ` std::declval<const T &>().fallback() ` is valid.
 * - The expression ` fallback(std::declval<const T &>()) ` is valid, where `fallback` can be found via
 * [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl).
 * @ingroup lib-effect
 */
template<typename T>
struct is_effect : lib::is_detected<lib::effect_result_t, T> {};

/**
 * @brief Provides operations for simulating scoped-algebraic effects (a.k.a. resumable exceptions.)
 * @ingroup lib-effect
 */
class effect {
private:
  static std::size_t next();

  template<typename T>
  static std::size_t index() {
    static const std::size_t output = next();
    return output;
  }

  struct entry {
    void *func;
    const std::vector<lib::optional<entry>> *context;
  };

  using context = std::vector<lib::optional<entry>>;

  static const context empty;
  static thread_local const context *current;

  struct clone_effect {
    lib::move_only_function<lib::finally_t<>(bool)> fallback() const;
  };

  template<typename Effect>
  struct base {
    static_assert(lib::is_effect<Effect>(), "Effect must satisfy is_effect");
    virtual lib::effect_result_t<Effect> operator()(Effect args) = 0;
  };

public:
  /**
   * @brief Invokes an effect.
   * @tparam T The type of effect to invoke.
   * @param args The arguments to the effect.
   * @return The result of invoking the effect.
   * @post If called before any handler has been installed, `lib::effect::invoke(args) == lib::fallback(args)`.
   */
  template<typename T, HALCHECK_REQUIRE(lib::is_effect<T>())>
  static lib::effect_result_t<T> invoke(T args) {
    const std::size_t i = index<T>();
    if (i < current->size() && (*current)[i]) {
      auto &entry = *(*current)[i];
      assert(entry.func && "entry.func should not be null");
      assert(entry.context && "entry.context should not be null");
      auto old = lib::exchange(current, entry.context);
      auto _ = lib::finally([&] { current = old; });
      return (*reinterpret_cast<base<T> *>(entry.func))(std::move(args));
    } else {
      auto old = lib::exchange(current, &empty);
      auto _ = lib::finally([&] { current = old; });
      return lib::fallback(std::move(args));
    }
  }

  /**
   * @brief Invokes an effect.
   * @tparam T The type of effect to invoke.
   * @tparam Args The types of arguments to pass to the effect.
   * @param args The arguments to pass to the effect.
   * @return The result of invoking the effect.
   * @post `effect::invoke<T>(args...) == effect::invoke(T{args...})`
   */
  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_effect<T>()),
      HALCHECK_REQUIRE(lib::is_brace_constructible<T, Args...>())>
  static lib::effect_result_t<T> invoke(Args &&...args) {
    return invoke(T{std::forward<Args>(args)...});
  }

  /**
   * @brief An effect @ref handler defines the behaviour of a set of effects.
   * @tparam Self The derived type (as used in
   * [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)).
   * @tparam Effects The set of effects to handle.
   */
  template<typename Self, typename... Effects>
  class handler : private base<clone_effect>, private base<Effects>... {
  private:
    class reset {
    private:
      friend class handler;

      context next;
      const context *old;

      explicit reset(handler *self) : next(self->install()), old(lib::exchange(current, &next)) {}

    public:
      reset(reset &&other) noexcept : next(std::move(other.next)), old(other.old) {
        assert(current == &other.next);
        current = &next;
      }

      reset(const reset &) = delete;
      reset &operator=(reset &&) = delete;
      reset &operator=(const reset &) = delete;
      ~reset() = default;

      void operator()() const { current = old; }
    };

    class owning_reset {
    private:
      friend class handler;

      Self self;
      context next;
      const context *old;

      explicit owning_reset(handler *self)
          : self(std::move(*static_cast<Self *>(self))), next(this->self.install()),
            old(lib::exchange(current, &next)) {}

    public:
      owning_reset(owning_reset &&other) noexcept(std::is_nothrow_move_constructible<Self>())
          : self(std::move(other.self)), next((current = other.old, self.install())),
            old(lib::exchange(current, &next)) {}

      owning_reset(const owning_reset &) = delete;
      owning_reset &operator=(owning_reset &&) = delete;
      owning_reset &operator=(const owning_reset &) = delete;
      ~owning_reset() = default;

      void operator()() const { current = old; }
    };

  public:
    /// @brief A type of object whose lifetime determines how long the set of effect handlers are overriden.
    using scope = lib::finally_t<reset>;

    /// @brief A type of object whose lifetime determines how long the set of effect handlers are overriden.
    using owning_scope = lib::finally_t<owning_reset>;

    /// @brief Overrides the behaviour of a set of effects.
    /// @return A value whose lifetime determines how long the set of effect handlers are overriden.
    /// @post While the return value is in scope, for all effects `T` and `args` of type `T`,
    /// - `effect::invoke(args) == (*this)(args)` if `T` is an element of @p Effects, and
    /// - `effect::invoke(args)` behaves as if it were called before calling this function otherwise.
    scope handle() & { return lib::finally(reset(this)); }

    /// @brief Overrides the behaviour of a set of effects.
    /// @return A value whose lifetime determines how long the set of effect handlers are overriden.
    /// @post While the return value is in scope, for all effects `T` and `args` of type `T`,
    /// - `effect::invoke(args) == (*this)(args)` if `T` is an element of @p Effects, and
    /// - `effect::invoke(args)` behaves as if it were called before calling this function otherwise.
    owning_scope handle() && { return lib::finally(owning_reset(this)); }

    /// @brief Invokes a function, handling any effects using this handler.
    /// @tparam F The type of function to execute.
    /// @tparam Args The type of arguments to pass to the function.
    /// @param func The function to execute.
    /// @param args The arguments to pass to the function.
    /// @return The result of invoking the function.
    /// @details This overload participates in overload resolution only if
    /// - `lib::is_invocable<F, Args...>()` holds, and
    /// - `lib::is_effect<T>()` holds for all types `T` in @p Effects.
    ///
    /// @post `this->handle(func, args...) == { auto _ = this->handle(); lib::invoke(func, args...); }`
    template<
        typename F,
        typename... Args,
        HALCHECK_REQUIRE(lib::is_invocable<F, Args...>()),
        HALCHECK_REQUIRE(lib::conjunction<lib::is_effect<Effects>...>())>
    lib::invoke_result_t<F, Args...> handle(F func, Args &&...args) {
      auto _ = handle();
      return lib::invoke(std::move(func), std::forward<Args>(args)...);
    }

  private:
    context install() {
      auto output = *current;
      output.resize(std::max({output.size(), index<clone_effect>(), index<Effects>()...}) + 1);
      output[index<clone_effect>()] = entry{static_cast<base<clone_effect> *>(this), current};
      lib::ignore = {(output[index<Effects>()] = entry{static_cast<base<Effects> *>(this), current})...};
      return output;
    }

    lib::effect_result_t<clone_effect> operator()(clone_effect) final {
      using namespace std::placeholders;
      return std::bind(
          [](bool owning, lib::effect_result_t<clone_effect> &outer, handler &copy) -> lib::finally_t<> {
            auto handler1 = outer(owning);
            if (owning) {
              return std::move(handler1) + std::move(copy).handle();
            } else {
              return std::move(handler1) + copy.handle();
            }
          },
          _1,
          invoke<clone_effect>(),
          *static_cast<Self *>(this));
    }
  };

  /**
   * @brief A @ref state determines the behaviour of all effects.
   */
  class state {
  public:
    /**
     * @brief Constructs the default state object.
     * @post <tt>
     * { auto _ = lib::effect::state().handle(); lib::effect::invoke(args); } ==
     * { auto _ = lib::effect::state().handle(); lib::fallback(args); }
     * </tt>
     */
    state() = default;

    /**
     * @brief Copies the current set of effect handlers.
     * @post Let `st` be a value of type @ref state constructed via `lib::effect::state(lib::in_place)`.
     * Then `{ auto _ = st; lib::effect::invoke(args); }` behaves as if `lib::effect::invoke(args)` is called at the
     * time `st` is constructed.
     */
    explicit state(lib::in_place_t);

    /**
     * @brief Overrides the current set of effect handlers.
     * @return A value whose lifetime determines how long the set of effect handlers are overriden.
     */
    lib::finally_t<> handle() &;

    /**
     * @brief Overrides the current set of effect handlers.
     * @return A value whose lifetime determines how long the set of effect handlers are overriden.
     */
    lib::finally_t<> handle() &&;

    /**
     * @brief Invokes a function, handling any effects using this @ref state.
     * @tparam F The type of function to execute.
     * @tparam Args The type of arguments to pass to the function.
     * @param func The function to execute.
     * @param args The arguments to pass to the function.
     * @return The result of invoking the function.
     * @details This overload participates in overload resolution only if
     * - `lib::is_invocable<F, Args...>()` holds, and
     * - `lib::is_effect<T>()` holds for all types `T` in @p Effects.
     *
     * @post `this->handle(func, args...) == { auto _ = this->handle(); lib::invoke(func, args...); }`
     */
    template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
    lib::invoke_result_t<F, Args...> handle(F func, Args &&...args) {
      auto _ = handle();
      return lib::invoke(std::move(func), std::forward<Args>(args)...);
    }

  private:
    lib::move_only_function<lib::finally_t<>(bool)> _impl;
  };

  /**
   * @brief Copies the current set of effect handlers.
   * @return `lib::effect::state(lib::in_place)`
   */
  static state save() { return state(lib::in_place); }
};

}} // namespace halcheck::lib

#endif
