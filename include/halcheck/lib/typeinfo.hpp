#ifndef HALCHECK_LIB_RTTI_HPP
#define HALCHECK_LIB_RTTI_HPP

/**
 * @defgroup lib-typeinfo lib/typeinfo
 * @brief Utilities for std::type_info.
 * @see https://en.cppreference.com/w/cpp/header/typeinfo
 * @ingroup lib
 */

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
#elif HALCHECK_DOXYGEN
#define HALCHECK_RTTI
#endif

#ifdef HALCHECK_RTTI
#include <string>
#include <typeinfo> // IWYU pragma: export
#endif

namespace halcheck { namespace lib {

#ifdef HALCHECK_RTTI
/**
 * @brief Gets the demangled name associated with a std::type_info.
 * @param info The std::type_info to extract a name from.
 * @return The std::string representation of info's name.
 * @ingroup lib-typeinfo
 */
std::string nameof(const std::type_info &info);

/**
 * @brief Gets the demangled name associated with a type.
 * @tparam T The type to extract a name from.
 * @return The std::string representation of the T's name.
 * @ingroup lib-typeinfo
 */
template<typename T>
std::string nameof() {
  return nameof(typeid(T));
}
#endif

/**
 * @brief A runtime type identifier that does not require RTTI.
 * @ingroup lib-typeinfo
 */
struct type_id {
public:
  /**
   * @brief Constructs the type identifier for `void`.
   */
  constexpr type_id() : _index(0) {}

  /**
   * @brief Gets a hash code associated with this type identifier.
   * @post <tt> x.hash() == y.hash() || x != y </tt>
   */
  std::size_t hash() const { return std::hash<std::size_t>()(_index); }

  /**
   * @brief Gets the unique type identifier associated with the given type.
   * @tparam T The type for which to obtain an identifier.
   * @returns A unique type identifier for @p T.
   * @post <tt> type_id::of<T>() == type_id::of<U>() </tt> iff <tt> std:is_same<T, U>() </tt>.
   * @details This overload only participates in overload resolution if @p T is not `void`.
   */
  template<typename T, HALCHECK_REQUIRE(!std::is_same<T, void>())>
  static type_id of() {
    static const type_id output(next());
    return output;
  }

  /**
   * @brief Gets the unique type identifier associated with the given type.
   * @tparam T The type for which to obtain an identifier.
   * @returns A unique type identifier for @p T.
   * @post <tt> type_id::of<T>() == type_id::of<U>() </tt> iff <tt> std:is_same<T, U>() </tt>.
   * @details This overload only participates in overload resolution if @p T is `void`.
   */
  template<typename T, HALCHECK_REQUIRE(std::is_same<T, void>())>
  static type_id of() {
    return {};
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
/**
 * @brief The std::hash specialization for @ref halcheck::lib::type_id.
 * @ingroup lib-typeinfo
 */
template<>
struct hash<halcheck::lib::type_id> {
  std::size_t operator()(halcheck::lib::type_id value) const { return value.hash(); }
};
} // namespace std
#endif
