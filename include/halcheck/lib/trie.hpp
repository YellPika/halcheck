#ifndef HALCHECK_LIB_TRIE_HPP
#define HALCHECK_LIB_TRIE_HPP

/// @file
/// @brief An implementation of a trie.
/// @see https://en.wikipedia.org/wiki/Trie

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace halcheck { namespace lib {

/// @brief An implementation of a trie. Semantically, this corresponds to a function @f$K* \to V@f$.
/// @see https://en.wikipedia.org/wiki/Trie
/// @tparam K The type used to index child nodes.
/// @tparam V The type of value stored in each node.
template<typename K, typename V, typename Hash = std::hash<K>>
class trie {
private:
  static_assert(lib::is_invocable_r<std::size_t, Hash, const K &>(), "Hash must be a hasher for K");
  static_assert(std::is_default_constructible<V>(), "V must be default constructible");

  std::shared_ptr<const V> _value;
  std::shared_ptr<std::unordered_map<K, trie>> _children;

public:
  /// @brief Constructs a trie where every node is assigned the same value.
  /// @post `*trie() == V()`
  /// @post `trie().drop(r) == trie()`
  trie() = default;

  /// @brief Constructs a trie with a given value and set of children.
  /// @param value The value to assign to the root node.
  /// @param begin A pointer to the beginning of a range of child trees.
  /// @param end A pointer to the end of a range of child trees.
  /// @post `*trie(x, xs) == x`
  /// @post `trie(x, xs).drop(k) == xs[k]`
  /// @post `trie(x, xs).drop({k, ...ks}) == xs[k].drop({...ks})`
  template<
      typename I,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, std::pair<const K, trie>>())>
  explicit trie(V value, I begin, I end)
      : _value(new V(std::move(value))), _children(new std::unordered_map<K, trie>(std::move(begin), std::move(end))) {}

  template<
      typename R,
      HALCHECK_REQUIRE(lib::is_input_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, std::pair<const K, trie>>())>
  explicit trie(V value, R &&range) : trie(std::move(value), lib::begin(range), lib::end(range)) {}

  /// @brief Gets the value associated with the root node of this trie.
  /// @return A reference to the value associated with the root node.
  const V &operator*() const {
    static const V fallback;
    if (_value)
      return *_value;
    else
      return fallback;
  }

  /// @brief Gets the value associated with a specific node.
  /// @tparam R The type of range holding a sequence of lookup keys.
  /// @param range A sequence of keys to look up.
  /// @return The value located at the node identified @p range.
  template<
      typename R,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, K>())>
  const V &operator[](const R &range) const {
    return *drop(range);
  }

  /// @brief Gets a specific subtree.
  /// @tparam R The type of range holding a sequence of keys.
  /// @param range A sequence of keys.
  /// @return The subtree identified by @p range.
  template<
      typename R,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, K>())>
  trie drop(const R &range) const {
    return drop(lib::begin(range), lib::end(range));
  }

  /// @brief Gets a specific subtree.
  /// @tparam I The type of iterator pointing to a sequence of keys.
  /// @param begin An iterator pointing to the beginning of a sequence of keys.
  /// @param end An iterator pointing one-past-the-end of a sequence of keys.
  /// @return The subtree identified by the range (@p begin, @p end].
  template<
      typename I,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, K>())>
  trie drop(I begin, I end) const {
    auto output = *this;
    for (; begin != end; ++begin) {
      auto it = output.find(*begin);
      if (it == output.end())
        return trie();
      else
        output = it->second;
    }

    return output;
  }

  /// @brief Gets a specific subtree.
  /// @param key The key identifying the desired subtree.
  /// @return The subtree identified by key.
  trie drop(const K &key) const {
    auto it = find(key);
    if (it == end())
      return trie();
    else
      return it->second;
  }

  /// @brief Constructs a new trie with a specific value replaced.
  /// @tparam R The type of range holding a sequence of keys.
  /// @param range A sequence of keys.
  /// @param value The value to assign.
  /// @return A new trie with the value of the node identified by @p range changed to @p value.
  template<
      typename I,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>, K>())>
  trie set(I begin, I end, V value) const {
    auto output = *this;
    auto current = &output;

    for (; begin != end; ++begin) {
      current->_children = current->_children ? std::make_shared<std::unordered_map<K, trie, Hash>>(*current->_children)
                                              : std::make_shared<std::unordered_map<K, trie, Hash>>();
      current = &(*current->_children)[*begin];
    }

    current->_value = std::make_shared<V>(std::move(value));
    return output;
  }

  /// @brief Constructs a new trie with a specific value replaced.
  /// @tparam R The type of range holding a sequence of keys.
  /// @param range A sequence of keys.
  /// @param value The value to assign.
  /// @return A new trie with the value of the node identified by @p range changed to @p value.
  template<
      typename R,
      HALCHECK_REQUIRE(lib::is_input_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, K>())>
  trie set(const R &range, V value) const {
    return set(lib::begin(range), lib::end(range), std::move(value));
  }

  class iterator : public lib::iterator_interface<iterator> {
  public:
    using value_type = typename std::unordered_map<K, trie, Hash>::value_type;
    using difference_type = typename std::unordered_map<K, trie, Hash>::difference_type;
    using reference = typename std::unordered_map<K, trie, Hash>::const_reference;
    using pointer = typename std::unordered_map<K, trie, Hash>::const_pointer;
    using iterator_category = std::forward_iterator_tag;

    using lib::iterator_interface<iterator>::operator++;

    iterator() = default;

    explicit iterator(typename std::unordered_map<K, trie, Hash>::const_iterator base) : _base(std::move(base)) {}

    iterator &operator++() {
      ++_base;
      return *this;
    }

    reference operator*() const { return *_base; }

    pointer operator->() const { return _base.operator->(); }

  private:
    friend bool operator==(const iterator &lhs, const iterator &rhs) { return lhs._base == rhs._base; }

    typename std::unordered_map<K, trie, Hash>::const_iterator _base;
  };

  /// @brief Returns an iterator to the first child.
  /// @return An iterator to the first child.
  iterator begin() const { return _children ? iterator(_children->begin()) : iterator(); }

  /// @brief Returns an iterator to one past the last child.
  /// @return An iterator to one past the last child.
  iterator end() const { return _children ? iterator(_children->end()) : iterator(); }

  /// @brief Returns the number of children in this trie.
  /// @return The number of children in this trie.
  std::size_t size() const { return _children.size(); }

  /// @brief Determines whether this trie has any children.
  /// @return `true` if and only if this trie has no children.
  bool empty() const { return _children.empty(); }

  iterator find(const K &key) const {
    if (_children)
      return iterator(_children->find(key));
    else
      return iterator();
  }
};

}} // namespace halcheck::lib

#endif
