#ifndef HALCHECK_LIB_ATOM_HPP
#define HALCHECK_LIB_ATOM_HPP

#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

namespace halcheck { namespace lib {

/// @brief A \ref symbol is conceptually a std::string with constant time equality comparison and hashing.
/// @ingroup lib
class symbol {
public:
  /// @brief Construct a new \ref symbol from a std::string.
  explicit symbol(const std::string &);

  /// @brief Construct a new \ref symbol from a C string.
  explicit symbol(const char * = "");

  /// @brief Gets a hash code for this \ref symbol. Equivalent to computing the hash code of the underlying std::string,
  ///        but guaranteed to be computed in constant time.
  ///
  /// @return The hash code of the associated std::string.
  inline std::size_t hash() const noexcept { return _data->second; }

  /// @brief Accesses the underlying std::string.
  ///
  /// @return A reference to the underlying std::string.
  /// @note Given x and y of type \ref symbol, then x == y iff &*x == &*y.
  const std::string &operator*() const noexcept;

private:
  /// @brief Determines if two symbols are equal.
  ///
  /// @param lhs,rhs The lib::symbols to compare.
  /// @retval true lhs and rhs' underlying strings are equal.
  /// @retval false lhs and rhs' underlying strings are not equal.
  friend bool operator==(const symbol &lhs, const symbol &rhs) { return lhs._data == rhs._data; }

  /// @brief Determines if two symbols are not equal.
  ///
  /// @param lhs,rhs The lib::symbols to compare.
  /// @retval true lhs and rhs' underlying strings are not equal.
  /// @retval false lhs and rhs' underlying strings are equal.
  friend bool operator!=(const symbol &lhs, const symbol &rhs) { return lhs._data != rhs._data; }

  std::pair<const std::string, std::size_t> *_data;
};

class number {
public:
  number() : _value(0) {}

  explicit number(std::uintmax_t value) : _value(value) {}

  std::uintmax_t &operator*() noexcept { return _value; }
  const std::uintmax_t &operator*() const noexcept { return _value; }

private:
  friend bool operator==(number lhs, number rhs) { return *lhs == *rhs; }
  friend bool operator!=(number lhs, number rhs) { return *lhs != *rhs; }
  friend bool operator<(number lhs, number rhs) { return *lhs < *rhs; }
  friend bool operator>(number lhs, number rhs) { return *lhs > *rhs; }
  friend bool operator<=(number lhs, number rhs) { return *lhs <= *rhs; }
  friend bool operator>=(number lhs, number rhs) { return *lhs >= *rhs; }

  std::uintmax_t _value;
};

/// @brief An atom is either a symbol or a number.
/// @ingroup lib
using atom = lib::variant<lib::symbol, lib::number>;

namespace literals {
#if __cplusplus >= 201806L
template<std::size_t N>
struct char_array {
  char data[N]; // NOLINT
  [[nodiscard]] constexpr size_t size() const { return N - 1; }
  constexpr char_array(const char (&init)[N]) { // NOLINT
    std::copy_n(init, N, data);
  }
};

template<literals::char_array Value>
lib::symbol operator""_s() {
  static lib::symbol output(std::string(Value.data, Value.size()));
  return output;
}
#elif (__cplusplus >= 201606L || defined(__clang__)) && defined(__GNUG__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
template<typename T, T... Data>
lib::symbol operator""_s() {
  static lib::symbol output(std::string{Data...});
  return output;
}
#pragma clang diagnostic pop
#else
inline lib::symbol operator""_s(const char *data, std::size_t size) { return lib::symbol(std::string(data, size)); }
#endif
inline lib::number operator""_n(unsigned long long int value) { return lib::number(value); }
} // namespace literals

}} // namespace halcheck::lib

namespace std {
template<>
struct hash<halcheck::lib::symbol> {
  std::size_t operator()(halcheck::lib::symbol value) const noexcept { return value.hash(); }
};
template<>
struct hash<halcheck::lib::number> {
  std::size_t operator()(halcheck::lib::number value) const noexcept { return std::hash<std::uintmax_t>()(*value); }
};
} // namespace std
#endif
