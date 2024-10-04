#ifndef HALCHECK_GEN_CONTAINER_HPP
#define HALCHECK_GEN_CONTAINER_HPP

#include <halcheck/gen/label.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace halcheck { namespace gen {

/// @brief Repeatedly calls a function. During shrinking, one or more calls
///        may be omitted.
/// @tparam F A nullary function type.
/// @param func The function to call.
template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom>())>
void repeat(lib::atom id, F func) {
  using namespace lib::literals;

  auto _ = gen::label(id);

  auto size = gen::sample("size"_s, gen::size());

  std::vector<bool> skip;
  {
    auto _ = gen::label("shrink"_s);
    for (std::uintmax_t i = 0; i < size; i++)
      skip.push_back(gen::shrink(lib::number(i)).has_value());
  }

  {
    auto _ = gen::label("func"_s);
    for (std::uintmax_t i = 0; i < size; i++) {
      if (!skip[skip.size() - i - 1])
        lib::invoke(func, lib::number(i));
    }
  }
}

/// @brief Generates a random container value.
/// @tparam T The type of container to generate.
/// @tparam F A nullary function type.
/// @param gen A function producing random values for the container.
/// @return A random container whose elements are drawn from gen.
template<
    typename T,
    typename F,
    HALCHECK_REQUIRE(lib::is_insertable_range<T>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<lib::range_value_t<T>, F, lib::atom>())>
T container(lib::atom id, F gen) {
  T output;
  auto it = lib::end(output);
  gen::repeat(id, [&](lib::atom id) { it = std::next(output.insert(it, lib::invoke(gen, id))); });
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
    HALCHECK_REQUIRE(lib::is_insertable_range<T>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<lib::range_value_t<T>, F, lib::atom>())>
T container(lib::atom id, std::size_t size, F gen) {
  auto _ = gen::label(id);

  T output;
  auto it = lib::end(output);
  while (size-- > 0)
    it = std::next(output.insert(it, lib::invoke(gen, lib::number(size))));
  return output;
}

template<typename I, HALCHECK_REQUIRE(lib::is_forward_iterator<I>())>
void shuffle(lib::atom id, I begin, I end) {
  auto _ = gen::label(id);
  for (auto it = begin; it != end; ++it)
    std::iter_swap(it, gen::range(lib::number(it - begin), it, end));
}

template<typename Range, HALCHECK_REQUIRE(lib::is_forward_range<lib::decay_t<Range>>())>
void shuffle(lib::atom id, Range &&range) {
  gen::shuffle(id, lib::begin(range), lib::end(range));
}

}} // namespace halcheck::gen

#endif
