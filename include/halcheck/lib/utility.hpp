#ifndef HALCHECK_LIB_UTILITY_HPP
#define HALCHECK_LIB_UTILITY_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <exception>
#include <sstream>
#include <string>
#include <utility> // IWYU pragma: export

namespace halcheck { namespace lib {

/// @brief Converts a value to a std::string using operator<<.
/// @tparam T The type of value to convert from.
/// @param value The value to convert from.
template<typename T>
std::string to_string(const T &value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

/// @brief Converts a std::string to a value using operator>>.
/// @tparam T The type of value to convert to.
/// @param value The string to convert from.
template<typename T>
lib::optional<T> of_string(const std::string &value) {
  T output;
  if (std::istringstream(value) >> output)
    return output;
  else
    return lib::nullopt;
}

lib::optional<std::string> getenv(const std::string &name);

template<typename T>
lib::optional<T> getenv(const std::string &name) {
  if (auto value = lib::getenv(name))
    return lib::of_string<T>(*value);
  else
    return lib::nullopt;
}

template<class T, class U = T>
T exchange(T &value, U &&next) {
  T prev = std::move(value);
  value = std::forward<U>(next);
  return prev;
}

template<typename T, T... Ints>
struct integer_sequence {};

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

template<typename T, T N>
struct make_integer_sequence : detail::make_integer_sequence<void, T, N> {};

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template<typename T>
class result_holder {
public:
  explicit result_holder(T value) : _value(std::move(value)) {}
  explicit result_holder(std::exception_ptr value) : _value(std::move(value)) {}

  T &get() & {
    return lib::visit(
        lib::overload(
            [](T &value) -> T & { return value; },
            [](std::exception_ptr e) -> T & { std::rethrow_exception(std::move(e)); }),
        _value);
  }

  const T &get() const & { return const_cast<result_holder *>(this)->get(); }

  explicit operator bool() const { return lib::holds_alternative<std::exception_ptr>(_value); }

private:
  lib::variant<T, std::exception_ptr> _value;
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
