#ifndef HALCHECK_LIB_UTILITY_HPP
#define HALCHECK_LIB_UTILITY_HPP

/// @file
/// @brief General utilities
/// @see https://en.cppreference.com/w/cpp/header/utility

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <exception>
#include <utility> // IWYU pragma: export

namespace halcheck { namespace lib {

/// @brief An implementation of std::in_place_t.
/// @see std::in_place_t
struct in_place_t {};

/// @brief An implementation of std::in_place.
/// @see std::in_place
static constexpr in_place_t in_place;

/// @brief An implementation of std::exchange.
/// @see std::exchange
template<typename T, typename U = T>
T exchange(T &value, U &&next) {
  T prev = std::move(value);
  value = std::forward<U>(next);
  return prev;
}

/// @brief An implementation of std::integer_sequence.
/// @see std::integer_sequence
template<typename T, T... Ints>
struct integer_sequence {};

/// @brief An implementation of std::index_sequence.
/// @see std::index_sequence
template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

namespace detail {
template<typename, typename T, T N, T... Ints>
struct make_integer_sequence;

template<typename T, T N, T... Ints>
struct make_integer_sequence<lib::enable_if_t<N == 0>, T, N, Ints...> : integer_sequence<T, Ints...> {};

template<typename T, T N, T... Ints>
struct make_integer_sequence<lib::enable_if_t<N != 0>, T, N, Ints...>
    : make_integer_sequence<void, T, N - 1, N - 1, Ints...> {};

} // namespace detail

/// @brief An implementation of std::make_integer_sequence.
/// @see std::make_integer_sequence
template<typename T, T N>
struct make_integer_sequence : detail::make_integer_sequence<void, T, N> {};

/// @brief An implementation of std::make_index_sequence.
/// @see std::make_index_sequence
template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<typename T>
class result_holder {
public:
  explicit result_holder(T value) : _value(std::move(value)) {}
  explicit result_holder(std::exception_ptr value) : _value(std::move(value)) {}

  T &get() & {
    return lib::visit(
        lib::make_overload(
            [](T &value) -> T & { return value; },
            [](std::exception_ptr e) -> T & { std::rethrow_exception(std::move(e)); }),
        _value);
  }

  const T &get() const & { return const_cast<result_holder *>(this)->get(); }

  explicit operator bool() const { return lib::holds_alternative<std::exception_ptr>(_value); }

private:
  lib::variant<T, std::exception_ptr> _value;
};

template<typename T>
class result_holder<T &> {
public:
  explicit result_holder(T &value) : _value(&value) {}
  explicit result_holder(std::exception_ptr value) : _value(std::move(value)) {}

  T &get() & {
    return lib::visit(
        lib::make_overload(
            [](T *value) -> T & { return *value; },
            [](std::exception_ptr e) -> T & { std::rethrow_exception(std::move(e)); }),
        _value);
  }

  const T &get() const & { return const_cast<result_holder *>(this)->get(); }

  explicit operator bool() const { return lib::holds_alternative<std::exception_ptr>(_value); }

private:
  lib::variant<T *, std::exception_ptr> _value;
};

template<>
class result_holder<void> {
public:
  explicit result_holder(std::exception_ptr value = nullptr) : _value(std::move(value)) {}

  void get() const {
    if (_value)
      std::rethrow_exception(_value);
  }

private:
  std::exception_ptr _value;
};

template<typename F, typename... Args, HALCHECK_REQUIRE(!std::is_void<lib::invoke_result_t<F, Args...>>())>
lib::result_holder<lib::invoke_result_t<F, Args...>> make_result_holder(F func, Args &&...args) {
  try {
    return lib::result_holder<lib::invoke_result_t<F, Args...>>(lib::invoke(func, std::forward<Args>(args)...));
  } catch (...) {
    return lib::result_holder<lib::invoke_result_t<F, Args...>>(std::current_exception());
  }
}

template<typename F, typename... Args, HALCHECK_REQUIRE(std::is_void<lib::invoke_result_t<F, Args...>>())>
lib::result_holder<void> make_result_holder(F func, Args &&...args) {
  try {
    lib::invoke(func, std::forward<Args>(args)...);
    return lib::result_holder<void>();
  } catch (...) {
    return lib::result_holder<void>(std::current_exception());
  }
}

}} // namespace halcheck::lib

#endif
