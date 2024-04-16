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

/// @brief Repeatedly calls a function. During shrinking, one or more calls may
///        may be omitted.
/// @tparam F A nullary function type.
/// @param func The function to call.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
void repeat(F func) {
  auto size = gen::size();
  gen::while_([&] {
    if (!gen::next(1, size--))
      return false;
    if (!gen::shrink())
      lib::invoke(func);
    return true;
  });
}

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
  T output;
  auto it = lib::end(output);
  gen::repeat([&] { it = std::next(output.insert(it, lib::invoke(gen))); });
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
  T output;
  auto it = lib::end(output);
  gen::while_([&] {
    if (size == 0)
      return false;

    --size;
    it = std::next(output.insert(it, lib::invoke(gen)));
    return true;
  });
  return output;
}

}} // namespace halcheck::gen

#endif
