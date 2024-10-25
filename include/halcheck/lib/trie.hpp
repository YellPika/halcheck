#ifndef HALCHECK_LIB_TRIE_HPP
#define HALCHECK_LIB_TRIE_HPP

/// @file
/// @brief An implementation of a trie.
/// @see https://en.wikipedia.org/wiki/Trie

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace halcheck { namespace lib {

/// @brief An implementation of a trie. Semantically, this corresponds to a function @f$K* \to V@f$.
/// @see https://en.wikipedia.org/wiki/Trie
/// @tparam K The type used to index child nodes.
/// @tparam V The type of value stored in each node.
template<
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    HALCHECK_REQUIRE(lib::is_invocable_r<std::size_t, Hash, const K &>()),
    HALCHECK_REQUIRE(std::is_default_constructible<V>())>
class trie {
public:
  /// @brief Constructs a trie where every node is assigned the same value.
  /// @post `*trie() == V()`
  /// @post `trie().drop(r) == trie()`
  trie() = default;

  /// @brief Constructs a trie with a given value and set of children.
  /// @param value The value to assign to the root node.
  /// @param children The child tries.
  /// @post `*trie(x, xs) == x`
  /// @post `trie(x, xs).drop(k) == xs[k]`
  /// @post `trie(x, xs).drop({k, ...ks}) == xs[k].drop({...ks})`
  explicit trie(V value, std::unordered_map<K, trie, Hash> children = {})
      : _value(std::make_shared<V>(std::move(value))),
        _children(std::make_shared<std::unordered_map<K, trie, Hash>>(std::move(children))) {}

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
    if (!_children)
      return trie();

    auto output = *this;
    for (auto &&key : range) {
      auto it = _children->find(key);
      if (it == _children->end())
        return trie();
      else
        output = it->second;
    }

    return output;
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
    return drop(lib::make_subrange(std::move(begin), std::move(end)));
  }

  /// @brief Gets a specific subtree.
  /// @param key The key identifying the desired subtree.
  /// @return The subtree identified by key.
  trie drop(const K &key) const {
    if (!_children)
      return trie();

    auto it = _children->find(key);
    if (it == _children->end())
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
      typename R,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>, K>())>
  trie set(const R &range, V value) const {
    auto output = *this;
    auto current = &output;

    for (auto &&key : range) {
      current->_children = current->_children ? std::make_shared<std::unordered_map<K, trie, Hash>>(*current->_children)
                                              : std::make_shared<std::unordered_map<K, trie, Hash>>();
      current = &(*current->_children)[key];
    }

    current->_value = std::make_shared<V>(std::move(value));
    return output;
  }

  /// @private
  const std::unordered_map<K, trie, Hash> &children() const {
    static std::unordered_map<K, trie, Hash> empty;
    return _children ? *_children : empty;
  }

private:
  std::shared_ptr<V> _value;
  std::shared_ptr<std::unordered_map<K, trie, Hash>> _children;
};

}} // namespace halcheck::lib

#endif
