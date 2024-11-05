#ifndef HALCHECK_LIB_DAG_HPP
#define HALCHECK_LIB_DAG_HPP

/**
 * @defgroup lib-dag lib/dag
 * @brief Labelled directed acyclic graphs.
 * @ingroup lib
 */

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

}} // namespace halcheck::lib

#endif
