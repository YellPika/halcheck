#ifndef HALCHECK_LIB_TREE_HPP
#define HALCHECK_LIB_TREE_HPP

#include <halcheck/lib/iterator/ranges.hpp>
#include <halcheck/lib/iterator/transform.hpp>
#include <halcheck/lib/memory.hpp>

#include <unordered_map>
#include <utility>

namespace halcheck { namespace lib {

template<typename K, typename V, template<typename...> class Map = std::unordered_map>
class tree {
private:
  struct unbox {
    std::pair<const K &, tree &> operator()(std::pair<const K, lib::box<tree>> &value) const {
      return {value.first, *value.second};
    }
  };

  struct const_unbox {
    std::pair<const K &, const tree &> operator()(const std::pair<const K, lib::box<tree>> &value) const {
      return {value.first, *value.second};
    }
  };

  V _value;
  Map<K, lib::box<tree>> _children;

public:
  tree &operator[](const K &key) { return *_children[key]; }

  tree &at(const K &key) { return *_children.at(key); }
  const tree &at(const K &key) const { return *_children.at(key); }

  V *get() {}
  const V *get() const {}

  V &operator*() { return *get(); }
  const V &operator*() const { return *get(); }

  V *operator->() { return get(); }
  const V *operator->() const { return get(); }

  using iterator = lib::transform_iterator<lib::iterator_t<Map<K, lib::box<tree>>>, unbox>;
  using const_iterator = lib::transform_iterator<lib::const_iterator_t<Map<K, lib::box<tree>>>, const_unbox>;

  iterator begin() { return lib::make_transform_iterator(_children.begin(), unbox()); }
  iterator end() { return lib::make_transform_iterator(_children.end(), unbox()); }

  const_iterator begin() const { return lib::make_transform_iterator(_children.begin(), const_unbox()); }
  const_iterator end() const { return lib::make_transform_iterator(_children.end(), const_unbox()); }

  const_iterator cbegin() const { return lib::make_transform_iterator(_children.begin(), const_unbox()); }
  const_iterator cend() const { return lib::make_transform_iterator(_children.end(), const_unbox()); }
};

}} // namespace halcheck::lib

#endif
