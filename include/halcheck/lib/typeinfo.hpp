#ifndef HALCHECK_LIB_RTTI_HPP
#define HALCHECK_LIB_RTTI_HPP

#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define HALCHECK_RTTI
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define HALCHECK_RTTI
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define HALCHECK_RTTI
#endif
#endif

#ifdef HALCHECK_RTTI
#include <string>
#include <typeinfo> // IWYU pragma: export
#endif

namespace halcheck { namespace lib {

#ifdef HALCHECK_RTTI
/// @brief Gets the demangled name associated with a std::type_info.
/// @param info The std::type_info to extract a name from.
/// @return The std::string representation of info's name.
std::string nameof(const std::type_info &info);

/// @brief Gets the demangled name associated with a type.
/// @tparam T The type to extract a name from.
/// @return The std::string representation of the T's name.
template<typename T>
std::string nameof() {
  return nameof(typeid(T));
}
#endif

struct type_id {
public:
  constexpr type_id() : _index(0) {}

  std::size_t hash() const { return std::hash<std::size_t>()(_index); }

  template<typename T, HALCHECK_REQUIRE(!std::is_void<T>())>
  static type_id make() {
    static type_id output(next());
    return output;
  }

  template<typename T, HALCHECK_REQUIRE(std::is_void<T>())>
  static type_id make() {
    return type_id();
  }

private:
  explicit constexpr type_id(std::size_t index) : _index(index) {}

  static std::size_t next();

  friend bool operator==(const type_id &lhs, const type_id &rhs) { return lhs._index == rhs._index; }
  friend bool operator!=(const type_id &lhs, const type_id &rhs) { return lhs._index != rhs._index; }
  friend bool operator<(const type_id &lhs, const type_id &rhs) { return lhs._index < rhs._index; }
  friend bool operator>(const type_id &lhs, const type_id &rhs) { return lhs._index > rhs._index; }
  friend bool operator<=(const type_id &lhs, const type_id &rhs) { return lhs._index <= rhs._index; }
  friend bool operator>=(const type_id &lhs, const type_id &rhs) { return lhs._index >= rhs._index; }

  std::size_t _index;
};

}} // namespace halcheck::lib

namespace std {
template<>
struct hash<halcheck::lib::type_id> {
  std::size_t operator()(halcheck::lib::type_id value) const { return value.hash(); }
};
} // namespace std
#endif
