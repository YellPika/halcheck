#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <set>

HALCHECK_TEST(DAG, Consistency) {
  using namespace halcheck;
  using namespace halcheck::lib::literals;

  using set = std::set<lib::dag<int>::const_iterator>;
  lib::dag<int> dag;
  auto gen_subset = [&](lib::atom id) {
    return dag.empty() ? set()
                       : gen::container<set>(id, [&](lib::atom id) { return gen::range(id, dag.begin(), dag.end()); });
  };

  for (auto _ : gen::repeat("init"_s)) {
    auto parents = gen_subset("parents"_s);
    dag.emplace(parents, gen::arbitrary<int>("label"_s));
  }

  set children;
  auto parents = gen_subset("parents"_s);
  auto it = dag.emplace(parents, gen::arbitrary<int>("label"_s));

  for (auto _ : gen::repeat("update"_s)) {
    auto parents = dag.empty() ? set() : gen::container<set>("parents"_s, [&](lib::atom id) {
      return gen::range(id, dag.begin(), dag.end());
    });
    auto j = dag.emplace(std::move(parents), gen::arbitrary<int>("label"_s));
    if (parents.count(it) > 0)
      children.insert(j);
  }

  auto it_parents = dag.parents(it);
  auto it_children = dag.children(it);
  EXPECT_EQ(parents, set(it_parents.begin(), it_parents.end()));
  EXPECT_EQ(children, set(it_children.begin(), it_children.end()));
}
