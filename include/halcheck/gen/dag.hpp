#ifndef HALCHECK_GEN_DAG_HPP
#define HALCHECK_GEN_DAG_HPP

/**
 * @defgroup gen-dag gen/dag
 * @brief Generators for lib::dag.
 * @ingroup gen
 */
#include <halcheck/gen/container.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/dag.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <unordered_map>
#include <vector>

namespace halcheck { namespace gen {

/**
 * @brief Generates a random lib::dag.
 * @tparam T The type of resource identifiers.
 * @tparam F The type of function used to generate labels.
 * @param id A unique identifier for the generated value.
 * @param func A function used to generate labels.
 * @return A random lib::dag whose labels are generated via @p func.
 * @details This function participates in overload resolution only if the following conditions hold:
 * 1. `lib::is_hashable<T>()` holds.
 * 2. `lib::is_equality_comparable<T>()` holds.
 * 3. `lib::is_invocable<F, lib::atom, lib::function_view<void(const T &)>>()` holds.
 *
 * The function @p func is passed a function `use`, which is used to indicate which nodes should have edges to the node
 * being generated. Formally, let @f$N(i)@f$ be the set of values passed to `use` during the generation of the @f$i@f$th
 * node. Node @f$i@f$ is connected (possibly indirectly) to node @f$j > i@f$ if @f$N(i) \cap N(j) \ne \emptyset@f$.
 * @ingroup gen-dag
 */
template<
    typename T,
    typename F,
    HALCHECK_REQUIRE(lib::is_hashable<T>()),
    HALCHECK_REQUIRE(lib::is_equality_comparable<T>()),
    HALCHECK_REQUIRE(lib::is_invocable<F, lib::atom, lib::function_view<void(const T &)>>())>
lib::dag<lib::invoke_result_t<F, lib::atom, lib::function_view<void(const T &)>>> dag(lib::atom id, F func) {
  using namespace lib::literals;
  using dag = lib::dag<lib::invoke_result_t<F, lib::atom, lib::function_view<void(const T &)>>>;
  using map = std::unordered_map<T, lib::iterator_t<dag>>;

  auto _ = gen::label(id);

  map state;
  dag output;

  for (auto _ : gen::repeat("labels"_s)) {
    std::vector<T> keys;
    std::vector<lib::iterator_t<dag>> parents;

    auto value = lib::invoke(func, "func"_s, [&](T key) {
      auto it = state.find(key);
      if (it != state.end())
        parents.push_back(it->second);
      keys.push_back(std::move(key));
    });

    auto it = output.emplace(std::move(parents), std::move(value));
    for (auto &&key : keys)
      state[key] = it;
  }

  return output;
}

}} // namespace halcheck::gen

#endif
