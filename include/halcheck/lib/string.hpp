#ifndef HALCHECK_LIB_STRING_HPP
#define HALCHECK_LIB_STRING_HPP

/// @file
/// @brief Additional string utilities.
/// @see https://en.cppreference.com/w/cpp/header/string

#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <sstream>
#include <string> // IWYU pragma: export

namespace halcheck { namespace lib {

/// @brief Converts a value to a std::string using operator<<.
/// @tparam T The type of value to convert from.
/// @param value The value to convert from.
/// @details This function can only be called if lib::is_printable<T> holds.
template<typename T, HALCHECK_REQUIRE(lib::is_printable<T>())>
std::string to_string(const T &value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

/// @brief Converts a std::string to a value using operator>>.
/// @tparam T The type of value to convert to.
/// @param value The string to convert from.
/// @details This function can only be called if @ref lib::is_parsable<T> holds.
template<typename T, HALCHECK_REQUIRE(lib::is_parsable<T>())>
lib::optional<T> of_string(const std::string &value) {
  T output;
  if (std::istringstream(value) >> output)
    return output;
  else
    return lib::nullopt;
}

/// @brief Gets the value of an environment variable.
/// @param name The name of the environment variable to get.
/// @return Returns lib::nullopt if the environment variable is not set. Otherwise, returns a std::string containing the
/// value of the environment variable.
lib::optional<std::string> getenv(const std::string &name);

/// @brief Gets the value of an environment variable.
/// @tparam T The type of value to read.
/// @param name The name of the environment variable to get.
/// @return Returns lib::nullopt if the environment variable is not set or the environment variable does not contain a
/// value of type @p T. Otherwise, returns a @p T containing the value of the environment variable.
template<typename T, HALCHECK_REQUIRE(lib::is_parsable<T>())>
lib::optional<T> getenv(const std::string &name) {
  if (auto value = lib::getenv(name))
    return lib::of_string<T>(*value);
  else
    return lib::nullopt;
}

}} // namespace halcheck::lib

#endif
