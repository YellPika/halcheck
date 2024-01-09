#ifndef HALCHECK_GEN_CONTAINER_HPP
#define HALCHECK_GEN_CONTAINER_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace halcheck { namespace gen {

/// @brief Generates a random container value.
/// @tparam T The type of container to generate.
/// @tparam F A nullary function type.
/// @param gen A function producing random values for the container.
/// @return A random container whose elements are drawn from gen.
template<
    typename T,
    typename F,
    HALCHECK_REQUIRE(lib::is_insertable<T>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<lib::range_value_t<T>, F>())>
T container(F gen) {
  auto _ = gen::group();
  auto size = gen::size();

  T output;
  auto it = lib::end(output);
  while (gen::next(1, size--)) {
    auto _ = gen::group();
    if (!gen::shrink())
      it = std::next(output.insert(it, lib::invoke(gen)));
  }
  return output;
}

/// @brief Generates a random container value.
/// @tparam T The type of container to generate.
/// @tparam F A nullary function type.
/// @param size The number of elements to generate.
/// @param gen A function producing random values for the container.
/// @return A random container whose elements are drawn from gen.
template<
    typename T,
    typename F,
    HALCHECK_REQUIRE(lib::is_insertable<T>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<lib::range_value_t<T>, F>())>
T container(std::size_t size, F gen) {
  auto _ = gen::group();

  T output;
  auto it = lib::end(output);
  while (size-- > 0) {
    auto _ = gen::group();
    it = std::next(output.insert(it, lib::invoke(gen)));
  }
  return output;
}

}} // namespace halcheck::gen

#endif
