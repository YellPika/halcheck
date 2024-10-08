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
#include <utility>
#include <vector>

namespace halcheck { namespace lib {

/// @brief Directed acyclic graphs with T-labelled nodes.
/// @tparam T The type of node label.
template<typename T>
class dag {
private:
  struct edges {
    std::vector<std::size_t> children;
    std::vector<std::size_t> parents;
  };

  std::vector<T> _labels;
  std::vector<edges> _edges;
  std::vector<std::size_t> _roots;

public:
  using iterator = lib::index_iterator<std::vector<T>>;
  using const_iterator = lib::index_iterator<const std::vector<T>>;
  static_assert(std::is_convertible<iterator, const_iterator>(), "");

  iterator begin() { return iterator(_labels); }
  const_iterator begin() const { return const_iterator(_labels); }

  iterator end() { return iterator(_labels, size()); }
  const_iterator end() const { return const_iterator(_labels, size()); }

  bool empty() const { return _labels.empty(); }

  std::size_t size() const { return _labels.size(); }

  template<
      typename I,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, const_iterator>()),
      HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(I begin, I end, Args &&...args) {
    auto index = size();
    _labels.emplace_back(std::forward<Args>(args)...);
    _edges.emplace_back();
    auto &parents = _edges.back().parents;

    try {
      if (begin == end)
        _roots.push_back(index);

      while (begin != end) {
        const_iterator i = *begin++;
        if (_edges[i.index()].children.empty() || _edges[i.index()].children.back() != index)
          _edges[i.index()].children.push_back(index);
        parents.push_back(i.index());
      }
    } catch (...) {
      for (auto &&edge : _edges) {
        if (edge.children.back() == index)
          edge.children.pop_back();
      }

      if (_roots.back() == index)
        _roots.pop_back();

      _labels.pop_back();
      _edges.pop_back();
      throw;
    }

    std::sort(parents.begin(), parents.end());
    parents.erase(std::unique(parents.begin(), parents.end()), parents.end());

    return iterator(_labels, index);
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
    _labels.clear();
    _edges.clear();
    _roots.clear();
  }

private:
  struct to_iterator {
    iterator operator()(std::size_t index) const { return iterator(self->_labels, index); }
    dag *self;
  };

  struct to_const_iterator {
    to_const_iterator() : self(nullptr) {};
    explicit to_const_iterator(const dag *self) : self(self) {}
    to_const_iterator(const to_iterator &other) : self(other.self) {} // NOLINT
    const_iterator operator()(std::size_t index) const { return const_iterator(self->_labels, index); }
    const dag *self;
  };

public:
  using view = lib::transform_view<lib::ref_view<const std::vector<std::size_t>>, to_iterator>;
  using const_view = lib::transform_view<lib::ref_view<const std::vector<std::size_t>>, to_const_iterator>;

  const_view children(const_iterator it) const {
    auto &&data = _edges[it.index()].children;
    return lib::transform(lib::ref(data), to_const_iterator{this});
  }

  view children(const_iterator it) {
    auto &&data = _edges[it.index()].children;
    return lib::transform(lib::ref(data), to_iterator{this});
  }

  const_view parents(const_iterator it) const {
    auto &&data = _edges[it.index()].parents;
    return lib::transform(lib::ref(data), to_const_iterator{this});
  }

  view parents(const_iterator it) {
    auto &&data = _edges[it.index()].parents;
    return lib::transform(lib::ref(data), to_iterator{this});
  }

  const_view roots() const { return lib::transform(lib::ref(_roots), to_const_iterator{this}); }

  view roots() { return lib::transform(lib::ref(_roots), to_iterator{this}); }

  void reserve(std::size_t size) {
    _labels.reserve(size);
    _edges.reserve(size);
  }
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
