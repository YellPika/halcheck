#ifndef HALCHECK_LIB_DAG_HPP
#define HALCHECK_LIB_DAG_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <algorithm>
#include <cstddef>
#include <future>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <vector>

namespace halcheck { namespace lib {

/// @brief Directed acyclic graphs with T-labelled nodes.
/// @tparam T The type of node label.
template<typename T>
class dag {
private:
  class node {
  public:
    template<typename... Args>
    explicit node(Args &&...args) : value(std::forward<Args>(args)...) {}

  private:
    friend class dag;

    T value;
    std::vector<std::size_t> children;
    std::vector<std::size_t> parents;
  };

  std::vector<node> _nodes;
  std::vector<std::size_t> _roots;

  struct from_node {
    T &operator()(node &node) const { return node.value; }
    const T &operator()(const node &node) const { return node.value; }
  };

  node &to_node(const T &value) { return const_cast<node &>(static_cast<const dag *>(this)->to_node(value)); }

  const node &to_node(const T &value) const {
    return *reinterpret_cast<const node *>(reinterpret_cast<const char *>(&value) - offsetof(node, value));
  }

  std::size_t to_index(const T &value) const { return &to_node(value) - _nodes.data(); }

public:
  using const_iterator = lib::transform_iterator<lib::index_iterator<const std::vector<node>>, from_node>;
  using iterator = lib::transform_iterator<lib::index_iterator<std::vector<node>>, from_node>;

  const_iterator begin() const { return const_iterator(lib::make_index_iterator(_nodes, 0), from_node()); }
  const_iterator end() const { return const_iterator(lib::make_index_iterator(_nodes, size()), from_node()); }
  iterator begin() { return iterator(lib::make_index_iterator(_nodes, 0), from_node()); }
  iterator end() { return iterator(lib::make_index_iterator(_nodes, size()), from_node()); }

  std::size_t size() const { return _nodes.size(); }

  template<
      typename I,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<typename std::iterator_traits<I>::reference, const_iterator>()),
      HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
  iterator emplace(I begin, I end, Args &&...args) {
    auto index = _nodes.size();
    _nodes.emplace_back(std::forward<Args>(args)...);
    auto &parents = _nodes.back().parents;

    try {
      if (begin == end)
        _roots.push_back(index);

      for (auto it = begin; it != end; ++it) {
        const_iterator i = *it;
        if (to_node(*i).children.empty() || to_node(*i).children.back() != index)
          to_node(*i).children.push_back(index);
        parents.push_back(to_index(*i));
      }
    } catch (...) {
      _nodes.pop_back();
      for (auto &&node : _nodes) {
        if (node.children.back() == index)
          node.children.pop_back();
      }
      throw;
    }

    std::sort(parents.begin(), parents.end());
    parents.erase(std::unique(parents.begin(), parents.end()), parents.end());

    return iterator(lib::make_index_iterator(_nodes, index), from_node());
  }

  template<
      typename R,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, const_iterator>()),
      HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
  iterator emplace(R &&range, Args &&...args) {
    return emplace(lib::begin(range), lib::end(range), std::forward<Args>(args)...);
  }

  template<typename... Args, HALCHECK_REQUIRE(lib::is_constructible<T, Args...>())>
  iterator emplace(const std::initializer_list<const_iterator> &range, Args &&...args) {
    return emplace(range.begin(), range.end(), std::forward<Args>(args)...);
  }

  void clear() {
    _nodes.clear();
    _roots.clear();
  }

private:
  struct to_iterator {
    iterator operator()(std::size_t index) const {
      return iterator(lib::make_index_iterator(*self, index), from_node());
    }
    std::vector<node> *self;
  };

  struct to_const_iterator {
    const_iterator operator()(std::size_t index) const {
      return const_iterator(lib::make_index_iterator(*self, index), from_node());
    }
    const std::vector<node> *self;
  };

public:
  using const_view = lib::transform_view<const std::vector<std::size_t>, to_const_iterator>;
  using view = lib::transform_view<std::vector<std::size_t>, to_iterator>;

  const_view children(const_iterator it) const {
    return lib::make_transform_view(to_node(*it).children, to_const_iterator{&_nodes});
  }

  view children(const_iterator it) { return lib::make_transform_view(to_node(*it).children, to_iterator{&_nodes}); }

  const_view parents(const_iterator it) const {
    return lib::make_transform_view(to_node(*it).parents, to_const_iterator{&_nodes});
  }

  view parents(const_iterator it) { return lib::make_transform_view(to_node(*it).parents, to_iterator{&_nodes}); }

  const_view roots() const { return lib::make_transform_view(_roots, to_const_iterator{&_nodes}); }

  view roots() { return lib::make_transform_view(_roots, to_iterator{&_nodes}); }

  void reserve(std::size_t size) { _nodes.reserve(size); }
};

/// @brief Executes a function on each value in a given graph and stores the
///        results in a graph with the same structure. Functions from unrelated
///        nodes are executed in parallel.
///
/// @tparam F The type of function to execute.
/// @tparam T The type of value stored in the graph.
/// @param graph The graph of functions to execute.
/// @param func The function to execute.
/// @return A graph with the same structure as the input graph.
template<typename F, typename T, HALCHECK_REQUIRE(lib::is_invocable<F, T &>())>
lib::dag<lib::invoke_result_t<F, T &>> async(lib::dag<T> &graph, F func) {
  std::vector<std::shared_future<void>> futures;
  futures.reserve(graph.size());

  std::vector<std::future<lib::invoke_result_t<F, T &>>> results;
  results.reserve(graph.size());

  for (auto i = graph.begin(); i != graph.end(); ++i) {
    std::vector<std::shared_future<void>> parents;
    for (auto j : graph.parents(i))
      parents.push_back(futures[j - graph.begin()]);

    std::promise<void> promise;
    futures.push_back(promise.get_future().share());
    results.push_back(std::async(
        std::launch::async,
        [&func, i](const std::vector<std::shared_future<void>> &parents, std::promise<void> promise) {
          auto _ = lib::finally([&] { promise.set_value(); });

          for (const auto &parent : parents)
            parent.wait();

          return lib::invoke(func, *i);
        },
        std::move(parents),
        std::move(promise)));
  }

  lib::dag<lib::invoke_result_t<F, T &>> output;
  output.reserve(graph.size());

  for (auto i = graph.begin(); i != graph.end(); ++i) {
    output.emplace(
        lib::make_transform_view(
            graph.parents(i),
            [&](lib::iterator_t<lib::dag<T>> j) { return output.begin() + (j - graph.begin()); }),
        results[i - graph.begin()].get());
  }
  return output;
}

/// @brief Executes a function on each value in a given graph and stores the
///        results in a graph with the same structure. Functions from unrelated
///        nodes are executed in parallel.
///
/// @tparam F The type of function to execute.
/// @tparam T The type of value stored in the graph.
/// @param graph The graph of functions to execute.
/// @param func The function to execute.
/// @return A graph with the same structure as the input graph.
template<typename F, typename T, HALCHECK_REQUIRE(lib::is_invocable<F, const T &>())>
lib::dag<lib::invoke_result_t<F, T &>> async(const lib::dag<T> &graph, F func) {
  return lib::async(const_cast<lib::dag<T> &>(graph), [&](const T &value) { return lib::invoke(func, value); });
}

}} // namespace halcheck::lib

#endif
