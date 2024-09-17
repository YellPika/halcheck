#ifndef HALCHECK_GEN_ELEMENT_HPP
#define HALCHECK_GEN_ELEMENT_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <array>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace halcheck { namespace gen {

static const struct {
  /// @brief Generates a random element of a container.
  /// @tparam T The type of container to draw elements from.
  /// @param container The container to draw elements from.
  /// @return A random element in the given container.
  /// @note This overload expects an r-value reference to a temporary. Thus,
  ///       it returns a (non-reference) value moved out of the range.
  template<
      typename T,
      HALCHECK_REQUIRE(lib::is_range<lib::decay_t<T>>()),
      HALCHECK_REQUIRE(!std::is_lvalue_reference<T>()),
      HALCHECK_REQUIRE(!std::is_const<lib::decay_t<T>>())>
  lib::range_value_t<lib::decay_t<T>> operator()(lib::atom id, T &&container) const {
    return std::move(*gen::range(id, lib::begin(container), lib::end(container)));
  }

  /// @brief Generates a random element of a container.
  /// @tparam T The type of container to draw elements from.
  /// @param container The container to draw elements from.
  /// @return A random element in the given container.
  /// @note This overload expects an l-value reference to a non-temporary value.
  ///       Thus, it returns a reference to a value in the range.
  template<typename T, HALCHECK_REQUIRE(lib::is_range<T>())>
  auto operator()(lib::atom id, T &container) const -> decltype(*lib::begin(container)) {
    return *gen::range(id, lib::begin(container), lib::end(container));
  }
} element_of;

static const struct {
  /// @brief Generates a random element of a std::initializer_list.
  /// @param args The elements to draw arguments from.
  /// @return A random element in the given std::initializer_list.
  template<typename... Args>
  lib::common_type_t<Args...> operator()(lib::atom id, Args &&...args) const {
    std::array<lib::common_type_t<Args...>, sizeof...(Args)> range{std::forward<Args>(args)...};
    return gen::element_of(id, std::move(range));
  }
} element;

static const struct {
  /// @brief Generates a random element of a container according to a weighted
  ///        distribution.
  /// @tparam T The type of container to draw elements from.
  /// @param container The container to draw elements from. The container should
  ///                  contain pairs, where every pair consists of a weight
  ///                  followed by a value.
  /// @return A random element in the given container.
  template<typename T, HALCHECK_REQUIRE(lib::is_range<T>())>
  auto operator()(lib::atom id, T &&container) const -> decltype(std::get<1>(*lib::begin(container))) {
    std::uintmax_t total = 0;
    for (auto &&pair : container)
      total += std::get<0>(pair);
    gen::guard(total > 0);

    auto random = gen::sample(id, total - 1);
    std::uintmax_t index = 0;
    for (auto &&pair : container) {
      auto weight = std::get<0>(pair);
      if (random < weight)
        break;

      random -= weight;
      ++index;
    }

    return std::get<1>(*std::next(lib::begin(container), gen::shrink_to(id, 0U, index)));
  }
} weighted_element_of;

// template<typename T>
// T weighted(const std::initializer_list<std::pair<std::uintmax_t, T>> &list) {
//   return gen::weighted<const std::initializer_list<std::pair<std::uintmax_t, T>> &>(list);
// }

}} // namespace halcheck::gen

#endif
