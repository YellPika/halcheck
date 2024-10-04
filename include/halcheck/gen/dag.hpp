#ifndef HALCHECK_GEN_DAG_HPP
#define HALCHECK_GEN_DAG_HPP

#include <halcheck/gen/container.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/dag.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace halcheck { namespace gen {

// static const struct dag_t : gen::labelable<dag_t> {
//   using gen::labelable<dag_t>::operator();

//   template<
//       typename F,
//       HALCHECK_REQUIRE(lib::is_invocable<F>()),
//       HALCHECK_REQUIRE(lib::is_tuple_like<lib::invoke_result_t<F>>()),
//       HALCHECK_REQUIRE(std::tuple_size<lib::invoke_result_t<F>>() == 2),
//       HALCHECK_REQUIRE(lib::is_forward_range<lib::tuple_element_t<0, lib::invoke_result_t<F>>>()),
//       HALCHECK_REQUIRE(lib::is_hashable<lib::range_value_t<lib::tuple_element_t<0, lib::invoke_result_t<F>>>>())>
//   lib::dag<lib::tuple_element_t<1, lib::invoke_result_t<F>>> operator()(F func) const {
//     using namespace lib::literals;

//     using key_t = lib::range_value_t<lib::tuple_element_t<0, lib::invoke_result_t<F>>>;
//     using value_t = lib::tuple_element_t<1, lib::invoke_result_t<F>>;

//     lib::dag<value_t> output;
//     std::unordered_map<key_t, lib::iterator_t<lib::dag<value_t>>> iterators;

//     gen::repeat([&] {
//       auto pair = lib::invoke(func);
//       auto &keys = std::get<0>(pair);
//       auto &value = std::get<1>(pair);

//       std::vector<lib::iterator_t<lib::dag<value_t>>> parents;
//       for (auto key : keys) {
//         auto it = iterators.find(key);
//         if (it != iterators.end())
//           parents.push_back(it->second);
//       }

//       auto it = output.emplace(parents, std::move(value));
//       for (auto key : keys)
//         iterators[key] = it;
//     });

//     return output;
//   }
// } dag;

}} // namespace halcheck::gen

#endif
