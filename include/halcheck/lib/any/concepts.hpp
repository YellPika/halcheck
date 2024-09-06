#ifndef HALCHECK_LIB_ANY_CONCEPTS_HPP
#define HALCHECK_LIB_ANY_CONCEPTS_HPP

#include <halcheck/lib/any/base.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>
#include <ostream>

namespace halcheck { namespace lib {

namespace detail {
class poly_deleter {
public:
  poly_deleter() : handle(nullptr) {}

  template<typename T>
  explicit poly_deleter(lib::in_place_type_t<T>) : handle([](void *ptr) { delete reinterpret_cast<T *>(ptr); }) {}

  void operator()(void *ptr) const {
    if (handle)
      handle(ptr);
  }

private:
  void (*handle)(void *);
};
} // namespace detail

template<typename...>
class unique_poly;

namespace concepts {

/// @brief A concept for copyable types.
class copyable : public virtual lib::concept_base {
public:
  copyable() = default;

  template<typename T, HALCHECK_REQUIRE(lib::is_copy_constructible<T>())>
  explicit copyable(lib::in_place_type_t<T>)
      : _copy([](const lib::concept_base &value) {
          return std::unique_ptr<void, detail::poly_deleter>(
              new T(value.as<T>()),
              detail::poly_deleter(lib::in_place_type_t<T>()));
        }) {}

private:
  template<typename...>
  friend class lib::unique_poly;

  std::unique_ptr<void, detail::poly_deleter> copy() const { return _copy ? _copy(*this) : nullptr; }
  std::unique_ptr<void, detail::poly_deleter> (*_copy)(const lib::concept_base &) = nullptr;
};

/// @brief A concept for types that can be printed.
/// @note This class should be inherited virtually.
class streamable : public virtual lib::concept_base {
public:
  streamable() = default;

  template<typename T, HALCHECK_REQUIRE(lib::is_streamable<T>())>
  explicit streamable(lib::in_place_type_t<T>)
      : _stream([](std::ostream &os, const streamable &value) -> std::ostream & { return os << value.as<T>(); }) {}

private:
  friend std::ostream &operator<<(std::ostream &os, const streamable &value) { return value._stream(os, value); }

  std::ostream &(*_stream)(std::ostream &, const streamable &) = nullptr;
};

/// @brief A concept for types with decidable equality.
class equality_comparable : public virtual lib::concept_base {
public:
  equality_comparable() = default;

  template<typename T, HALCHECK_REQUIRE(lib::is_equality_comparable<T>())>
  explicit equality_comparable(lib::in_place_type_t<T>)
      : _equal([](const equality_comparable &lhs, const equality_comparable &rhs) {
          return lhs.type() == rhs.type() && lhs.as<T>() == rhs.as<T>();
        }) {}

private:
  friend bool operator==(const equality_comparable &lhs, const equality_comparable &rhs) {
    return lhs._equal(lhs, rhs);
  }

  friend bool operator!=(const equality_comparable &lhs, const equality_comparable &rhs) {
    return !lhs._equal(lhs, rhs);
  }

  bool (*_equal)(const equality_comparable &, const equality_comparable &) = nullptr;
};

/// @brief A concept for types with decidable equality.
class totally_ordered : public virtual lib::concept_base {
public:
  totally_ordered() = default;

  template<typename T, HALCHECK_REQUIRE(lib::is_totally_ordered<T>())>
  explicit totally_ordered(lib::in_place_type_t<T>)
      : _lt([](const totally_ordered &lhs, const totally_ordered &rhs) {
          return lhs.type() < rhs.type() || (lhs.type() == rhs.type() && lhs.as<T>() < rhs.as<T>());
        }),
        _le([](const totally_ordered &lhs, const totally_ordered &rhs) {
          return lhs.type() < rhs.type() || (lhs.type() == rhs.type() && lhs.as<T>() <= rhs.as<T>());
        }) {}

private:
  friend bool operator<(const totally_ordered &lhs, const totally_ordered &rhs) { return lhs._lt(lhs, rhs); }

  friend bool operator<=(const totally_ordered &lhs, const totally_ordered &rhs) { return lhs._le(lhs, rhs); }

  bool (*_lt)(const totally_ordered &, const totally_ordered &) = nullptr;
  bool (*_le)(const totally_ordered &, const totally_ordered &) = nullptr;
};

} // namespace concepts
}} // namespace halcheck::lib

#endif
