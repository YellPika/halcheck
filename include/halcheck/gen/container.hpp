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

static const struct repeat_t : gen::labelable<repeat_t> {
  using gen::labelable<repeat_t>::operator();

  /// @brief Repeatedly calls a function. During shrinking, one or more calls
  ///        may be omitted.
  /// @tparam F A nullary function type.
  /// @param func The function to call.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    using namespace lib::literals;

    auto size = gen::sample("size"_s, gen::size());

    std::vector<bool> skip;
    for (std::uintmax_t i = 0; i < size; i++)
      skip.push_back(gen::shrink("shrink"_s, lib::number(i)).has_value());

    for (std::uintmax_t i = 0; i < size; i++) {
      if (!skip[skip.size() - i - 1])
        gen::label("func"_s, lib::number(i), func);
    }
  }
} repeat;

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
  while (size-- > 0)
    gen::label(lib::number(size), [&] { it = std::next(output.insert(it, lib::invoke(gen))); });
  return output;
}

static const struct shuffle_t : gen::labelable<shuffle_t> {
  using gen::labelable<shuffle_t>::operator();

  template<typename I, HALCHECK_REQUIRE(lib::is_forward_iterator<I>())>
  void operator()(I begin, I end) const {
    for (auto it = begin; it != end; ++it)
      std::iter_swap(it, gen::range(lib::number(it - begin), it, end));
  }

  template<typename Range, HALCHECK_REQUIRE(lib::is_forward_range<lib::decay_t<Range>>())>
  void operator()(Range &&range) const {
    (*this)(lib::begin(range), lib::end(range));
  }
} shuffle;

}} // namespace halcheck::gen

#endif
