#ifndef HALCHECK_LIB_ANY_HPP
#define HALCHECK_LIB_ANY_HPP

#include <halcheck/lib/any/base.hpp>     // IWYU pragma: export
#include <halcheck/lib/any/concepts.hpp> // IWYU pragma: export
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/typeinfo.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <exception>
#include <memory>
#include <ostream>
#include <type_traits>

namespace halcheck { namespace lib {

template<typename... Concepts>
class unique_poly : public virtual lib::concept_base, public Concepts... {
public:
  template<typename...>
  friend class unique_poly;

  unique_poly() = default;
  unique_poly(unique_poly &&) noexcept = default;
  unique_poly(const unique_poly &) = delete;
  unique_poly &operator=(unique_poly &&) noexcept = default;
  unique_poly &operator=(const unique_poly &) = delete;
  ~unique_poly() = default;

  /// @brief Construct a new unique poly object by moving from another.
  /// @tparam T The type of object to move from.
  /// @param other The object to move from.
  template<
      typename T,
      HALCHECK_REQUIRE(!std::is_lvalue_reference<T>()),
      HALCHECK_REQUIRE(!std::is_const<lib::remove_reference_t<T>>()),
      HALCHECK_REQUIRE(std::is_base_of<lib::concept_base, lib::decay_t<T>>()),
      HALCHECK_REQUIRE(lib::conjunction<std::is_base_of<Concepts, lib::decay_t<T>>...>())>
  unique_poly(T &&other) noexcept // NOLINT: implicit move from any concept type
      : Concepts(other)..., _type(other.type()), _impl(std::move(other._impl)) {}

  /// @brief Construct a new unique poly object by copying from another.
  /// @tparam T The type of object to copy from.
  /// @param other The object to copy from.
  template<
      typename T,
      HALCHECK_REQUIRE(std::is_base_of<lib::concepts::copyable, lib::decay_t<T>>()),
      HALCHECK_REQUIRE(lib::conjunction<std::is_base_of<Concepts, lib::decay_t<T>>...>())>
  unique_poly(const T &other) noexcept // NOLINT: implicit conversion from any concept type
      : Concepts(other)..., _type(other.type()), _impl(static_cast<const lib::concepts::copyable &>(other).copy()) {}

  /// @brief Construct a new unique poly object in place.
  /// @tparam T The type of object to store.
  /// @tparam Args... The types of arguments to construct the object with.
  /// @param tag Indicates the type of object to construct.
  /// @param args The arguments used to construct the object.
  template<
      typename T,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_constructible<T, Args...>()),
      HALCHECK_REQUIRE(lib::conjunction<lib::is_satisfied<Concepts, T>...>())>
  explicit unique_poly(lib::in_place_type_t<T> tag, Args &&...args)
      : Concepts(tag)..., _type(lib::type_id::make<T>()),
        _impl(new T(std::forward<Args>(args)...), detail::poly_deleter(tag)) {}

  /// @brief Construct a new unique poly object from a value.
  /// @tparam T The type of value to store.
  /// @param value The value to store.
  template<
      typename T,
      HALCHECK_REQUIRE(lib::is_constructible<lib::decay_t<T>, T &&>()),
      HALCHECK_REQUIRE(!std::is_base_of<lib::concept_base, lib::decay_t<T>>()),
      HALCHECK_REQUIRE(lib::conjunction<lib::is_satisfied<Concepts, lib::decay_t<T>>...>())>
  unique_poly(T &&value) // NOLINT: implicit conversion from any non-concept type
      : unique_poly(lib::in_place_type_t<lib::decay_t<T>>(), std::forward<T>(value)) {}

  void swap(unique_poly &other) {
    using std::swap;
    lib::ignore{(swap(static_cast<Concepts &>(*this), static_cast<Concepts &>(other)), 0)...};
    swap(_type, other._type);
    swap(_impl, other._impl);
  }

  bool has_value() const { return bool(_impl); }

  explicit operator bool() const { return bool(_impl); }

  void reset() { _impl.reset(); }

  template<typename T, typename... Args, HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
  T &emplace(Args &&...args) {
    auto output = new T(std::forward<Args>(args)...);
    _impl.reset(output);
    _impl.get_deleter() = detail::poly_deleter(lib::in_place_type_t<T>());
    _type = lib::type_id::make<T>();
    return *output;
  }

  lib::type_id type() const noexcept override { return _type; }

private:
  void *get() noexcept override { return _impl.get(); }

  lib::type_id _type;
  std::unique_ptr<void, detail::poly_deleter> _impl;
};

template<typename... Concepts>
class poly : public lib::unique_poly<lib::concepts::copyable, Concepts...> {
private:
  using base = lib::unique_poly<lib::concepts::copyable, Concepts...>;

public:
  using base::base;

  poly() = default;
  poly(poly &&) = default;
  poly &operator=(poly &&) = default;
  ~poly() = default;

  poly(const poly &other) : base(other) {}

  poly &operator=(const poly &other) {
    if (this != &other)
      this->base::operator=(base(other));
    return *this;
  }
};

using unique_any = lib::unique_poly<>;

using any = lib::poly<>;

using unique_equality_comparable = lib::unique_poly<lib::concepts::equality_comparable>;

using equality_comparable = lib::poly<lib::concepts::equality_comparable>;

template<typename T, typename... Concepts, typename... Args>
lib::unique_poly<Concepts...> make_unique_poly(Args &&...args) {
  return lib::unique_poly<Concepts...>(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

template<typename T, typename... Concepts, typename... Args>
lib::poly<Concepts...> make_poly(Args &&...args) {
  return lib::poly<Concepts...>(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

template<typename T, typename... Args, HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
lib::unique_any make_unique_any(Args &&...args) {
  return lib::make_unique_poly<T>(std::forward<Args>(args)...);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_copy_constructible<T>()),
    HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
lib::any make_any(Args &&...args) {
  return lib::make_poly<T>(std::forward<Args>(args)...);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_equality_comparable<T>()),
    HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
lib::unique_equality_comparable make_unique_equality_comparable(Args &&...args) {
  return lib::make_unique_poly<T, lib::concepts::equality_comparable>(std::forward<Args>(args)...);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_copy_constructible<T>()),
    HALCHECK_REQUIRE(lib::is_equality_comparable<T>()),
    HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
lib::equality_comparable make_equality_comparable(Args &&...args) {
  return lib::make_poly<T, lib::concepts::equality_comparable>(std::forward<Args>(args)...);
}

}} // namespace halcheck::lib

#endif
