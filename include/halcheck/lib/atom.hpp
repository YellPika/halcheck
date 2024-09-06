#ifndef HALCHECK_LIB_ATOM_HPP
#define HALCHECK_LIB_ATOM_HPP

#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

namespace halcheck { namespace lib {

class symbol {
public:
  explicit symbol(const std::string &);
  explicit symbol(const char * = "");

  inline std::size_t hash() const noexcept { return _data->second; }

  const std::string &operator*() const noexcept;

private:
  friend bool operator==(const symbol &lhs, const symbol &rhs) { return lhs._data == rhs._data; }
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

using atom = lib::variant<lib::symbol, lib::number>;

namespace literals {
inline lib::symbol operator""_s(const char *data, std::size_t size) { return lib::symbol(std::string(data, size)); }
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
