#ifndef HALCHECK_GEN_ELEMENT_HPP
#define HALCHECK_GEN_ELEMENT_HPP

#include <halcheck/gen/next.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <vector>

namespace halcheck { namespace gen {

/// @brief Generates a random element of a container.
/// @tparam T The type of container to draw elements from.
/// @param container The container to draw elements from.
/// @return A random element in the given container.
template<typename T, HALCHECK_REQUIRE(lib::is_range<T>())>
auto element(T &&container) -> decltype(*lib::begin(container)) {
  return *gen::range(lib::begin(container), lib::end(container));
}

/// @brief Generates a random element of a std::initializer_list.
/// @tparam T The type of element to generate.
/// @param list The std::initializer_list to draw elements from.
/// @return A random element in the given std::initializer_list.
template<typename T>
T element(const std::initializer_list<T> &list) {
  return *gen::range(list.begin(), list.end());
}

/// @brief Generates a random element of a container according to a weighted
///        distribution.
/// @tparam T The type of container to draw elements from.
/// @param container The container to draw elements from. The container should
///                  contain pairs, where every pair consists of a weight
///                  followed by a value.
/// @return A random element in the given container.
template<typename T, HALCHECK_REQUIRE(lib::is_range<T>())>
auto weighted(T &&container) -> decltype(std::get<1>(*lib::begin(container))) {
  auto _ = gen::group();

  std::vector<gen::weight> weights = {0};
  for (auto &&pair : container)
    weights.push_back(weights.back() + pair.first);

  std::size_t min = 0, max = weights.size() - 1;
  while (min + 1 < max) {
    auto mid = lib::midpoint(min, max);
    if (gen::next(weights[mid] - weights[min], weights[max] - weights[mid]))
      min = mid;
    else
      max = mid;
  }

  return std::get<1>(*std::next(lib::begin(container), gen::shrink.to(0U, min)));
}

template<typename T>
T weighted(const std::initializer_list<std::pair<gen::weight, T>> &list) {
  return gen::weighted<const std::initializer_list<std::pair<gen::weight, T>> &>(list);
}

}} // namespace halcheck::gen

#endif
