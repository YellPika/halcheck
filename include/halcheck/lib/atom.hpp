#ifndef HALCHECK_LIB_ATOM_HPP
#define HALCHECK_LIB_ATOM_HPP

/// @file
/// @brief Identifier types with constant time equality comparison and hashing.
/// @par Example
/// @code
/// using namespace halcheck;
/// using namespace halcheck::lib::literals;
/// lib::atom x = "example 1"_s;
/// lib::atom y = "example 2"_s;
/// lib::atom z = "example 1"_s;
/// lib::atom w = 3;
/// assert(x != y);
/// assert(x == z);
/// assert(x != w);
/// @endcode

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace halcheck { namespace lib {

/// @brief A \ref symbol is conceptually a std::string with constant time equality comparison and hashing.
class symbol {
public:
  using value_type = std::string;

  /// @brief Construct a new \ref symbol from a std::string.
  /// @post <tt> (const std::string &)symbol(x) == x </tt>
  /// @note This constructor has O(n) complexity, where n is the length of the input string. Prefer using the string
  /// literal version whenever possible. For example:
  /// \code
  /// using namespace halcheck::lib::literals;
  /// symbol example = "my symbol"_s;
  /// \endcode
  explicit symbol(const std::string &);

  /// @brief Construct a new \ref symbol from a C string.
  /// @post <tt> symbol(x) == symbol(std::string(x)) </tt>
  explicit symbol(const char * = "");

  /// @brief Gets a hash code for this \ref symbol. Equivalent to computing the hash code of the underlying std::string,
  ///        but guaranteed to be computed in constant time.
  ///
  /// @return The hash code of the associated std::string.
  /// @post <tt> x.hash() == std::hash<std::string>()((const std::string &)x) </tt>
  inline std::size_t hash() const noexcept { return _data->second; }

  /// @brief Casts the underlying string to a value.
  ///
  /// @tparam T The type of value to cast to.
  /// @return The underlying string reference casted to \p T.
  /// @post <tt> (T)x == (T)(const std::string &)x </tt>
  template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, const std::string &>())>
  explicit operator T() const {
    return T(_data->first);
  }

private:
  /// @brief Determines if two symbols are equal.
  ///
  /// @param lhs,rhs The \ref symbol "symbols" to compare.
  /// @retval true The underlying strings of \p lhs and \p rhs are equal.
  /// @retval false The underlying strings of \p lhs and \p rhs are not equal.
  /// @post <tt> x == y </tt> if and only if <tt> (const std::string &)x == (const std::string &)y </tt>
  friend bool operator==(const symbol &lhs, const symbol &rhs) { return lhs._data == rhs._data; }

  /// @brief Determines if two symbols are not equal.
  ///
  /// @param lhs,rhs The \ref symbol "symbols" to compare.
  /// @retval true The underlying strings of \p lhs and \p rhs are not equal.
  /// @retval false The underlying strings of \p lhs and \p rhs are equal.
  /// @post <tt> x != y </tt> if and only if <tt> (const std::string &)x != (const std::string &)y </tt>
  friend bool operator!=(const symbol &lhs, const symbol &rhs) { return lhs._data != rhs._data; }

  std::pair<const std::string, std::size_t> *_data;
};

/// @brief A \ref number is conceptually a signed integer with constant time equality comparison and hashing.
class number {
public:
  /// @brief The underlying integral type.
  using value_type = std::int64_t;

  /// @brief Creates a number with zero as its underlying value.
  /// @post <tt> (const value_type &)number() == 0 </tt>
  number() = default;

  /// @brief Creates a number with the given underlying value.
  /// @post <tt> (const value_type &)number(x) == x </tt>
  number(value_type value) // NOLINT: implicit conversion
      : _value(value) {}

  /// @brief Casts the underlying number to a value.
  ///
  /// This overload participates in overload resolution only if
  /// + <tt> std::is_integral<T>() </tt> holds,
  /// + <tt> std::is_signed<T>() </tt> holds, and
  /// + <tt> sizeof(T) >= sizeof(\ref value_type) </tt>
  ///
  /// @tparam T The type of value to cast to.
  /// @return The underlying number reference casted to \p T.
  /// @post <tt> (T)x == (T)(const value_type &)x </tt>
  template<
      typename T,
      HALCHECK_REQUIRE(std::is_integral<T>()),
      HALCHECK_REQUIRE(std::is_signed<T>()),
      HALCHECK_REQUIRE(sizeof(T) >= sizeof(value_type))>
  explicit operator T() const {
    return _value;
  }

  /// @brief Casts the underlying number to a value.
  ///
  /// This overload participates in overload resolution only if
  /// + <tt> std::is_integral<T>() </tt> does not hold, and
  /// + <tt> std::is_constructible<T, \ref value_type>() </tt> holds.
  ///
  /// @tparam T The type of value to cast to.
  /// @return The underlying number reference casted to \p T.
  /// @post <tt> (T)x == T((const value_type &)x) </tt>
  template<
      typename T,
      HALCHECK_REQUIRE(!std::is_integral<T>()),
      HALCHECK_REQUIRE(std::is_constructible<T, const value_type &>())>
  explicit operator T() const {
    return T(_value);
  }

private:
  /// @brief Determines if two numbers are equal.
  ///
  /// @param lhs,rhs The \ref number "numbers" to compare.
  /// @retval true The underlying strings of \p lhs and \p rhs are equal.
  /// @retval false The underlying strings of \p lhs and \p rhs are not equal.
  /// @post <tt> x == y </tt> if and only if <tt> (const value_type &)x == (const value_type &)y </tt>
  friend bool operator==(number lhs, number rhs) { return lhs._value == rhs._value; }

  /// @brief Determines if two numbers are equal.
  ///
  /// @param lhs,rhs The \ref number "numbers" to compare.
  /// @retval true The underlying strings of \p lhs and \p rhs are not equal.
  /// @retval false The underlying strings of \p lhs and \p rhs are equal.
  /// @post <tt> x != y </tt> if and only if <tt> (const value_type &)x != (const value_type &)y </tt>
  friend bool operator!=(number lhs, number rhs) { return lhs._value != rhs._value; }

  value_type _value = 0;
};

/// @brief An atom is either a symbol or a number.
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
#elif ((__cplusplus >= 201606L || defined(__clang__)) && defined(__GNUG__)) || defined(HALCHECK_DOXYGEN)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
/// @brief String literal constructor for lib::symbol.
/// @return A @ref symbol representing the same string as given by the string literal.
template<typename T, T... Data>
lib::symbol operator""_s() {
  static_assert(std::is_same<T, char>(), "T should be char");
  static lib::symbol output(std::string{Data...});
  return output;
}
#pragma clang diagnostic pop
#else
inline lib::symbol operator""_s(const char *data, std::size_t size) { return lib::symbol(std::string(data, size)); }
#endif
} // namespace literals

}} // namespace halcheck::lib

namespace std {

/// @brief The std::hash specialization for @ref halcheck::lib::symbol.
template<>
struct hash<halcheck::lib::symbol> {
  std::size_t operator()(halcheck::lib::symbol value) const noexcept { return value.hash(); }
};

/// @brief The std::hash specialization for @ref halcheck::lib::number.
template<>
struct hash<halcheck::lib::number> {
  std::size_t operator()(halcheck::lib::number value) const noexcept {
    return std::hash<halcheck::lib::number::value_type>()(halcheck::lib::number::value_type(value));
  }
};

} // namespace std
#endif
