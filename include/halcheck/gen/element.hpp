#ifndef HALCHECK_GEN_ELEMENT_HPP
#define HALCHECK_GEN_ELEMENT_HPP

/**
 * @defgroup gen-element gen/element
 * @brief Generating values in a range.
 * @ingroup gen
 */

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <array>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

namespace halcheck { namespace gen {

/**
 * @brief Generates a random element of a range.
 * @par Signature
 * @code
 * template<typename T>
 * lib::range_reference_t<T> element_of(lib::atom id, T &&range) // (1)
 *
 * template<typename T>
 * lib::range_value_t<T> element_of(lib::atom id, T &&range)     // (2)
 * @endcode
 * @tparam T The type of range to generate an element from.
 * @param id A unique id for the generated element.
 * @param range The range to generate an element from.
 * @return A random element of @p range.
 * @details A call to this function is well-formed only if one of the following is true:
 * - `lib::is_input_range<T>()` and `lib::is_sized_range<T>()` hold.
 * - `lib::is_forward_range<T>()` holds.
 *
 * Overload (1) is invoked if @p T is an l-value reference and `lib::is_forward_range<T>` holds. Otherwise, overload (2)
 * is invoked.
 *
 * @note Overload (1) is called if @p T is an l-value reference and `lib::is_forward_range<T>` does not hold because the
 * value obtained from an input iterator is not guaranteed to exist once the iterator is modified or destroyed.
 * @ingroup gen-element
 */
static const struct {
  template<typename T, HALCHECK_REQUIRE(lib::is_input_range<T>()), HALCHECK_REQUIRE(lib::is_sized_range<T>())>
  lib::conditional_t<lib::is_forward_range<T>::value, lib::range_reference_t<T>, lib::range_value_t<T>>
  operator()(lib::atom id, T &range) const {
    auto it = lib::begin(range);
    auto index = gen::range(id, 0, lib::size(range));
    std::advance(it, index);
    return *it;
  }

  template<typename T, HALCHECK_REQUIRE(lib::is_forward_range<T>()), HALCHECK_REQUIRE(!lib::is_sized_range<T>())>
  lib::range_reference_t<T> operator()(lib::atom id, T &range) const {
    return *gen::range(id, lib::begin(range), lib::end(range));
  }

  template<
      typename T,
      HALCHECK_REQUIRE(
          (lib::is_input_range<lib::remove_reference_t<T>>() && lib::is_sized_range<lib::remove_reference_t<T>>()) ||
          lib::is_forward_range<lib::remove_reference_t<T>>()),
      HALCHECK_REQUIRE(!std::is_lvalue_reference<T>())>
  lib::range_value_t<lib::remove_reference_t<T>> operator()(lib::atom id, T &&range) const {
    return std::move((*this)(id, range));
  }
} element_of;

/**
 * @brief Generates a random value from a fixed list of elements.
 * @par Signature
 * @code
 * template<typename... Args>
 * lib::common_type_t<Args...> element(lib::atom id, Args &&...args)
 * @endcode
 * @tparam Args The types of elements to draw values from.
 * @param id A unique identifier for the generated value.
 * @param args The elements to draw values from.
 * @return A random element of @p args.
 * @ingroup gen-element
 */
static const struct {
  template<typename... Args>
  lib::common_type_t<Args...> operator()(lib::atom id, Args &&...args) const {
    std::array<lib::common_type_t<Args...>, sizeof...(Args)> range{std::forward<Args>(args)...};
    return gen::element_of(id, std::move(range));
  }
} element;

// TODO: weighted_element_of, weighted_element

}} // namespace halcheck::gen

#endif
