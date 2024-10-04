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
#include <utility>
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

  struct from_index {
    T &operator()(std::size_t index) const { return (*nodes)[index].value; }
    std::vector<node> *nodes;
  };
  struct from_const_index {
    from_const_index() : nodes(nullptr) {}
    explicit from_const_index(const std::vector<node> *nodes) : nodes(nodes) {}
    from_const_index(from_index other) : nodes(other.nodes) {} // NOLINT
    const T &operator()(std::size_t index) const { return (*nodes)[index].value; }
    const std::vector<node> *nodes;
  };

public:
  using iterator = lib::transform_iterator<lib::iota_iterator<std::size_t>, from_index>;
  using const_iterator = lib::transform_iterator<lib::iota_iterator<std::size_t>, from_const_index>;
  static_assert(std::is_convertible<iterator, const_iterator>(), "");

  iterator begin() { return iterator(lib::iota_iterator<std::size_t>(0), from_index{&_nodes}); }
  const_iterator begin() const { return const_iterator(lib::iota_iterator<std::size_t>(0), from_const_index{&_nodes}); }

  iterator end() { return iterator(lib::iota_iterator<std::size_t>(size()), from_index{&_nodes}); }
  const_iterator end() const {
    return const_iterator(lib::iota_iterator<std::size_t>(size()), from_const_index{&_nodes});
  }

  std::size_t size() const { return _nodes.size(); }

  template<
      typename I,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, const_iterator>()),
      HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(I begin, I end, Args &&...args) {
    auto index = _nodes.size();
    _nodes.emplace_back(std::forward<Args>(args)...);
    auto &parents = _nodes.back().parents;

    try {
      if (begin == end)
        _roots.push_back(index);

      while (begin != end) {
        const_iterator i = *begin++;
        if (_nodes[*i.base()].children.empty() || _nodes[*i.base()].children.back() != index)
          _nodes[*i.base()].children.push_back(index);
        parents.push_back(*i.base());
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

    return iterator(lib::make_iota_iterator(index), from_index{&_nodes});
  }

  template<
      typename R,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, const_iterator>()),
      HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(R &&range, Args &&...args) {
    return emplace(lib::begin(range), lib::end(range), std::forward<Args>(args)...);
  }

  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
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
      return iterator(lib::make_iota_iterator((*data)[index]), from_index{nodes});
    }
    std::vector<node> *nodes;
    std::vector<std::size_t> *data;
  };

  struct to_const_iterator {
    to_const_iterator() : nodes(nullptr), data(nullptr) {};
    to_const_iterator(const std::vector<node> *nodes, const std::vector<std::size_t> *data)
        : nodes(nodes), data(data) {}
    to_const_iterator(to_iterator other) : nodes(other.nodes), data(other.data) {} // NOLINT
    const_iterator operator()(std::size_t index) const {
      return const_iterator(lib::make_iota_iterator((*data)[index]), from_const_index{nodes});
    }
    const std::vector<node> *nodes;
    const std::vector<std::size_t> *data;
  };

public:
  using view = lib::subrange<lib::transform_iterator<lib::iota_iterator<std::size_t>, to_iterator>>;
  using const_view = lib::subrange<lib::transform_iterator<lib::iota_iterator<std::size_t>, to_const_iterator>>;

  const_view children(const_iterator it) const {
    auto &&data = _nodes[*it.base()].children;
    return const_view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_const_iterator{&_nodes, &data}),
        lib::make_transform_iterator(lib::make_iota_iterator(data.size()), to_const_iterator{&_nodes, &data}));
  }

  view children(const_iterator it) {
    auto &&data = _nodes[*it.base()].children;
    return view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_iterator{&_nodes, &data}),
        lib::make_transform_iterator(lib::make_iota_iterator(data.size()), to_iterator{&_nodes, &data}));
  }

  const_view parents(const_iterator it) const {
    auto &&data = _nodes[*it.base()].parents;
    return const_view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_const_iterator{&_nodes, &data}),
        lib::make_transform_iterator(lib::make_iota_iterator(data.size()), to_const_iterator{&_nodes, &data}));
  }

  view parents(const_iterator it) {
    auto &&data = _nodes[*it.base()].parents;
    return view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_iterator{&_nodes, &data}),
        lib::make_transform_iterator(lib::make_iota_iterator(data.size()), to_iterator{&_nodes, &data}));
  }

  const_view roots() const {
    return const_view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_const_iterator{&_nodes, &_roots}),
        lib::make_transform_iterator(lib::make_iota_iterator(_roots.size()), to_const_iterator{&_nodes, &_roots}));
  }

  view roots() {
    return view(
        lib::make_transform_iterator(lib::make_iota_iterator(std::size_t(0)), to_iterator{&_nodes, &_roots}),
        lib::make_transform_iterator(lib::make_iota_iterator(_roots.size()), to_iterator{&_nodes, &_roots}));
  }

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
    auto f = [&](lib::iterator_t<lib::dag<T>> j) { return output.begin() + (j - graph.begin()); };
    auto parents = graph.parents(i);
    output.emplace(
        lib::make_transform_iterator(parents.begin(), f),
        lib::make_transform_iterator(parents.end(), f),
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
