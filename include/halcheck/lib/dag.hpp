#ifndef HALCHECK_LIB_DAG_HPP
#define HALCHECK_LIB_DAG_HPP

/**
 * @defgroup lib-dag lib/dag
 * @brief Labelled directed acyclic graphs.
 * @ingroup lib
 */

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <algorithm>
#include <cstddef>
#include <future>
#include <initializer_list>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace halcheck { namespace lib {

/**
 * @brief Directed acyclic graphs with labelled nodes.
 * @tparam T The type of node label.
 * @ingroup lib-dag
 */
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
  /**
   * @brief The type of node labels.
   */
  using value_type = T;

  /**
   * @brief A iterator to @p T. These are also referred to as "nodes".
   */
  using iterator = lib::index_iterator<std::vector<T>>;

  /**
   * @brief An iterator to `const` @p T. These are also referred to as "nodes".
   */
  using const_iterator = lib::index_iterator<const std::vector<T>>;

  static_assert(std::is_convertible<iterator, const_iterator>(), "");

  dag() = default;

  template<
      typename R,
      typename F,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, T>()),
      HALCHECK_REQUIRE(lib::is_range<lib::invoke_result_t<F, lib::range_reference_t<R>>>()),
      HALCHECK_REQUIRE(lib::is_hashable<lib::range_value_t<lib::invoke_result_t<F, lib::range_reference_t<R>>>>()),
      HALCHECK_REQUIRE(
          lib::is_equality_comparable<lib::range_value_t<lib::invoke_result_t<F, lib::range_reference_t<R>>>>())>
  dag(R &&range, F func) {
    using resource = lib::range_value_t<lib::invoke_result_t<F, lib::range_reference_t<R>>>;
    std::unordered_map<resource, const_iterator> state;
    for (auto &&value : range) {
      const auto &resources = func(value);
      std::vector<const_iterator> parents;
      for (auto &&resource : resources) {
        auto it = state.find(resource);
        if (it != state.end()) {
          parents.push_back(std::move(it->second));
          state.erase(it);
        }
      }

      auto it = emplace(std::move(parents), value);
      for (auto &&resource : resources) {
        auto i = state.find(resource);
        if (i == state.end())
          state.emplace(std::move(resource), it);
        else
          i->second = it;
      }
    }
  }

  /**
   * @brief Gets an @ref dag<T>::iterator "iterator" pointing to the first node in this @ref dag.
   * @return The @ref dag<T>::iterator "iterator" pointing to the first node in this @ref dag.
   */
  iterator begin() { return iterator(_labels); }

  /**
   * @brief Gets an @ref const_iterator pointing to the first node in this @ref dag.
   * @return The @ref const_iterator pointing to the first node in this @ref dag.
   */
  const_iterator begin() const { return const_iterator(_labels); }

  /**
   * @brief Gets an @ref dag<T>::iterator "iterator" pointing past the last node in this @ref dag.
   * @return The @ref dag<T>::iterator "iterator" pointing past the last node in this @ref dag.
   */
  iterator end() { return iterator(_labels, size()); }

  /**
   * @brief Gets an @ref const_iterator pointing past the last node in this @ref dag.
   * @return The @ref const_iterator pointing past the last node in this @ref dag.
   */
  const_iterator end() const { return const_iterator(_labels, size()); }

  /**
   * @brief Determines whether this @ref dag is empty.
   * @retval true This @ref dag contains no nodes.
   * @retval false This @ref dag contains at least one node.
   */
  bool empty() const { return _labels.empty(); }

  /**
   * @brief Gets the number of nodes in this @ref dag.
   * @return The number of nodes in this @ref dag.
   */
  std::size_t size() const { return _labels.size(); }

  /**
   * @brief Adds a node to this @ref dag with its label constructed in place.
   * @tparam I The type of iterator pointing within a collection of parent nodes.
   * @tparam Args The type of arguments used to construct the node's label.
   * @param first An iterator pointing to the first parent node.
   * @param last An iterator pointing past the last parent node.
   * @param args The arguments used to construct the node's label.
   * @return An iterator pointing to the newly constructed node.
   */
  template<
      typename I,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, const_iterator>()),
      HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(I first, I last, Args &&...args) {
    auto index = size();
    _labels.emplace_back(std::forward<Args>(args)...);
    _edges.emplace_back();
    auto &parents = _edges.back().parents;

    try {
      if (first == last)
        _roots.push_back(index);

      while (first != last) {
        const_iterator i = *first++;
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

  /**
   * @brief Adds a node to this @ref dag with its label constructed in place.
   * @tparam R The type of range containing a collection of parent nodes.
   * @tparam Args The type of arguments used to construct the node's label.
   * @param range The range containing a collection of parent nodes.
   * @param args The arguments used to construct the node's label.
   * @return An iterator pointing to the newly constructed node.
   */
  template<
      typename R,
      typename... Args,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, const_iterator>()),
      HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(R &&range, Args &&...args) {
    return emplace(lib::begin(range), lib::end(range), std::forward<Args>(args)...);
  }

  /**
   * @brief Adds a node to this @ref dag with its label constructed in place.
   * @tparam Args The type of arguments used to construct the node's label.
   * @param range The range containing a collection of parent nodes.
   * @param args The arguments used to construct the node's label.
   * @return An iterator pointing to the newly constructed node.
   */
  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  iterator emplace(const std::initializer_list<const_iterator> &range, Args &&...args) {
    return emplace(range.begin(), range.end(), std::forward<Args>(args)...);
  }

  /**
   * @brief Removes all nodes from this @ref dag.
   * @note This invalidates all @ref iterator "iterator"s belonging to this @ref dag.
   */
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
  /**
   * @brief A collection of @ref iterator "iterator"s.
   */
  using view = lib::transform_view<lib::ref_view<const std::vector<std::size_t>>, to_iterator>;

  /**
   * @brief A collection of @ref const_iterator "const_iterator"s.
   */
  using const_view = lib::transform_view<lib::ref_view<const std::vector<std::size_t>>, to_const_iterator>;

  /**
   * @brief Obtains a node's children.
   * @param it The node whose children should be queried.
   * @return The set of nodes that have @p it as a parent.
   */
  const_view children(const_iterator it) const {
    auto &&data = _edges[it.index()].children;
    return lib::transform(lib::ref(data), to_const_iterator{this});
  }

  /**
   * @brief Obtains a node's children.
   * @param it The node whose children should be queried.
   * @return The set of nodes that have @p it as a parent.
   */
  view children(const_iterator it) {
    auto &&data = _edges[it.index()].children;
    return lib::transform(lib::ref(data), to_iterator{this});
  }

  /**
   * @brief Obtains a node's parents.
   * @param it The node whose parents should be queried.
   * @return The set of nodes that have @p it as a child.
   */
  const_view parents(const_iterator it) const {
    auto &&data = _edges[it.index()].parents;
    return lib::transform(lib::ref(data), to_const_iterator{this});
  }

  /**
   * @brief Obtains a node's parents.
   * @param it The node whose parents should be queried.
   * @return The set of nodes that have @p it as a child.
   */
  view parents(const_iterator it) {
    auto &&data = _edges[it.index()].parents;
    return lib::transform(lib::ref(data), to_iterator{this});
  }

  /**
   * @brief Obtains the root nodes of this @ref dag.
   * @return The set of root nodes.
   */
  const_view roots() const { return lib::transform(lib::ref(_roots), to_const_iterator{this}); }

  /**
   * @brief Obtains the root nodes of this @ref dag.
   * @return The set of root nodes.
   */
  view roots() { return lib::transform(lib::ref(_roots), to_iterator{this}); }

  /**
   * @brief Reserves enough memory to contain the specified number of nodes.
   * @param size The number of nodes to reserve space for.
   */
  void reserve(std::size_t size) {
    _labels.reserve(size);
    _edges.reserve(size);
  }
};

/**
 * @brief Executes a function on each label in a given graph. Calls for unrelated node labels are executed in parallel.
 * @tparam F The type of function to execute.
 * @tparam T The type of label stored in the graph.
 * @param graph The graph of functions to execute.
 * @param func The function to execute.
 */
template<
    typename F,
    typename T,
    HALCHECK_REQUIRE(std::is_void<lib::invoke_result_t<F, lib::iterator_t<lib::dag<T>>>>())>
void async(lib::dag<T> &graph, F func) {
  std::vector<std::shared_future<void>> futures;
  futures.reserve(graph.size());

  for (auto i = graph.begin(); i != graph.end(); ++i) {
    std::vector<std::shared_future<void>> parents;
    for (auto j : graph.parents(i))
      parents.push_back(futures[j - graph.begin()]);

    futures.emplace_back(std::async(
        std::launch::async,
        [&func, i](const std::vector<std::shared_future<void>> &parents) {
          for (const auto &parent : parents)
            parent.wait();

          lib::invoke(func, i);
        },
        std::move(parents)));
  }

  for (auto &&future : futures)
    future.get();
}

/**
 * @brief Executes a function on each label in a given graph. Calls for unrelated node labels are executed in parallel.
 * @tparam F The type of function to execute.
 * @tparam T The type of label stored in the graph.
 * @param graph The graph of functions to execute.
 * @param func The function to execute.
 */
template<
    typename F,
    typename T,
    HALCHECK_REQUIRE(std::is_void<lib::invoke_result_t<F, lib::iterator_t<const lib::dag<T>>>>())>
void async(const lib::dag<T> &graph, F func) {
  return lib::async(const_cast<lib::dag<T> &>(graph), [&](lib::iterator_t<const lib::dag<T>> it) {
    return lib::invoke(func, it);
  });
}

/**
 * @brief Executes a function on each label in a given graph and stores the results in a graph with the same structure.
 * Calls for unrelated nodes are executed in parallel.
 * @tparam F The type of function to execute.
 * @tparam T The type of label stored in the graph.
 * @param graph The graph of functions to execute.
 * @param func The function to execute.
 * @return A graph with the same structure as the input graph.
 */
template<
    typename F,
    typename T,
    HALCHECK_REQUIRE(!std::is_void<lib::invoke_result_t<F, lib::iterator_t<lib::dag<T>>>>())>
lib::dag<lib::invoke_result_t<F, lib::iterator_t<lib::dag<T>>>> async(lib::dag<T> &graph, F func) {
  using result = lib::invoke_result_t<F, lib::iterator_t<lib::dag<T>>>;

  std::vector<std::promise<result>> results(graph.size());
  lib::async(graph, [&](lib::iterator_t<lib::dag<T>> it) {
    results[it - graph.begin()].set_value(lib::invoke(func, it));
  });

  lib::dag<result> output;
  output.reserve(graph.size());

  for (std::size_t i = 0; i < results.size(); i++) {
    output.emplace(
        lib::transform(
            graph.parents(graph.begin() + i),
            [&](lib::iterator_t<lib::dag<T>> it) { return output.begin() + (it - graph.begin()); }),
        results[i].get_future().get());
  }

  return output;
}

/**
 * @brief Executes a function on each label in a given graph and stores the results in a graph with the same structure.
 * Calls for unrelated nodes are executed in parallel.
 * @tparam F The type of function to execute.
 * @tparam T The type of label stored in the graph.
 * @param graph The graph of functions to execute.
 * @param func The function to execute.
 * @return A graph with the same structure as the input graph.
 */
template<
    typename F,
    typename T,
    HALCHECK_REQUIRE(!std::is_void<lib::invoke_result_t<F, lib::iterator_t<const lib::dag<T>>>>())>
lib::dag<lib::invoke_result_t<F, lib::iterator_t<const lib::dag<T>>>> async(const lib::dag<T> &graph, F func) {
  return lib::async(const_cast<lib::dag<T> &>(graph), [&](lib::iterator_t<const lib::dag<T>> it) {
    return lib::invoke(func, it);
  });
}

namespace detail {
template<typename T, typename S, typename F>
bool linearize(
    const lib::dag<T> &dag,
    S &seed,
    F func,
    std::vector<std::size_t> &references,
    std::vector<typename lib::dag<T>::const_iterator> &queue) {
  for (std::size_t i = 0; i < queue.size(); ++i) {
    auto next = seed;
    auto it = queue[i];
    if (!lib::invoke(func, *it, next))
      continue;

    for (auto j : dag.children(it)) {
      if (--references[j - dag.begin()] == 0)
        queue.push_back(j);
    }

    std::swap(queue[i], queue.back());
    queue.pop_back();

    auto _ = lib::finally([&] {
      queue.push_back(it);
      std::swap(queue[i], queue.back());

      for (auto j : dag.children(it)) {
        if (references[j - dag.begin()]++ == 0)
          queue.pop_back();
      }
    });

    if (linearize(dag, next, func, references, queue)) {
      seed = std::move(next);
      return true;
    }
  }

  return queue.empty();
}
} // namespace detail

template<
    typename T,
    typename S,
    typename F,
    HALCHECK_REQUIRE(lib::is_copyable<S>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<bool, F, const T &, S &>())>
bool linearize(const lib::dag<T> &dag, S &seed, F func) {
  std::vector<std::size_t> references(dag.size(), 0);
  for (auto i = dag.begin(); i != dag.end(); ++i) {
    for (auto j : dag.children(i))
      ++references[j - dag.begin()];
  }

  std::vector<typename lib::dag<T>::const_iterator> queue(dag.roots().begin(), dag.roots().end());
  return detail::linearize(dag, seed, func, references, queue);
}

template<
    typename T,
    typename F,
    typename G,
    HALCHECK_REQUIRE(lib::is_movable<lib::invoke_result_t<F>>()),
    HALCHECK_REQUIRE(lib::is_invocable_r<bool, G, const T &, lib::invoke_result_t<F> &>())>
lib::optional<lib::invoke_result_t<F>> linearize(const lib::dag<T> &dag, F init, G func) {
  struct state {
  private:
    std::reference_wrapper<F> init;
    std::reference_wrapper<G> func;
    std::vector<std::reference_wrapper<const T>> prefix;
    lib::optional<lib::invoke_result_t<F>> value;

  public:
    explicit state(F &init, G &func) : init(init), func(func) {}

    state(state &&other) noexcept(lib::is_nothrow_swappable<lib::optional<lib::invoke_result_t<F>>>())
        : init(other.init), func(other.func), prefix(std::move(other.prefix)) {
      using std::swap;
      swap(value, other.value);
    }

    state &operator=(state &&other) noexcept(lib::is_nothrow_swappable<lib::optional<lib::invoke_result_t<F>>>()) {
      if (this != &other) {
        init = other.init;
        func = other.func;
        prefix = std::move(other.prefix);

        using std::swap;
        value.reset();
        swap(value, other.value);
      }

      return *this;
    }

    state(const state &other) : init(other.init), func(other.func), prefix(other.prefix) {}

    state &operator=(const state &other) {
      if (this != &other) {
        init = other.init;
        func = other.func;
        prefix = other.prefix;
      }

      return *this;
    }

    ~state() = default;

    bool apply(const T &label) {
      if (func(label, get())) {
        prefix.push_back(label);
        return true;
      } else {
        value.reset();
        return false;
      }
    }

    lib::invoke_result_t<F> &get() {
      if (!value) {
        value.emplace(lib::invoke(init));
        for (auto &&label : prefix) {
          if (!func(label, *value))
            throw std::runtime_error("Failed to reproduce move-only value");
        }
      }

      return *value;
    }
  };

  state seed(init, func);
  if (lib::linearize(dag, seed, [&](const T &label, state &current) { return current.apply(label); }))
    return std::move(seed.get());
  else
    return lib::nullopt;
}

}} // namespace halcheck::lib

#endif
