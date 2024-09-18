#ifndef HALCHECK_LIB_ANY_BASE_HPP
#define HALCHECK_LIB_ANY_BASE_HPP

#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/typeinfo.hpp>
#include <halcheck/lib/variant.hpp>

#include <exception>

namespace halcheck { namespace lib {

/// @brief The base class for all concepts.
/// @note This class should be inherited virtually.
class concept_base {
public:
  /// @brief Gets the type ID of the contained object.
  /// @return An ID unique to the contained object's type.
  virtual lib::type_id type() const noexcept { std::terminate(); }

  /// @brief Gets the contained object.
  /// @tparam T The type of object that should be contained.
  /// @return A reference to the contained object.
  template<typename T>
  T &as() noexcept {
    return *reinterpret_cast<T *>(get());
  }

  /// @brief Gets the contained object.
  /// @tparam T The type of object that should be contained.
  /// @return A reference to the contained object.
  template<typename T>
  const T &as() const noexcept {
    return *reinterpret_cast<const T *>(get());
  }

protected:
  virtual void *get() noexcept { std::terminate(); }

  const void *get() const noexcept { return const_cast<concept_base *>(this)->get(); }
};

template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, const lib::remove_cvref_t<T> &>())>
T poly_cast(const concept_base &value) {
  return value.as<lib::remove_cvref_t<T>>();
}

template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cvref_t<T> &>())>
T poly_cast(concept_base &value) {
  return value.as<lib::remove_cvref_t<T>>();
}

template<typename T, HALCHECK_REQUIRE(std::is_constructible<T, lib::remove_cvref_t<T>>())>
T poly_cast(concept_base &&value) {
  return value.as<lib::remove_cvref_t<T>>();
}

/// @brief Indicates whether a type satisfies a concept.
/// @tparam Concept The concept to check.
/// @tparam T The type to check against the concept.
template<typename Concept, typename T, HALCHECK_REQUIRE(std::is_base_of<lib::concept_base, Concept>())>
struct is_satisfied : std::is_constructible<Concept, lib::in_place_type_t<T>> {};

}} // namespace halcheck::lib

#endif
