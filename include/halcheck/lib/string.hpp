#ifndef HALCHECK_LIB_STRING_HPP
#define HALCHECK_LIB_STRING_HPP

#include <halcheck/lib/optional.hpp>

#include <sstream>
#include <string>
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

}} // namespace halcheck::lib

#endif
