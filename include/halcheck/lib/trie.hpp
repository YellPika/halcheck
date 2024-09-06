#ifndef HALCHECK_LIB_TRIE_HPP
#define HALCHECK_LIB_TRIE_HPP

#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/memory.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>

namespace halcheck { namespace lib {

template<typename K, typename V>
class trie {
private:
  struct data {
    V value;
    std::unordered_map<K, trie> children;
    std::mutex mutex;
  };

  std::shared_ptr<data> _data;

public:
  explicit trie(V value = V(), std::unordered_map<K, trie> children = {})
      : _data(new data{std::move(value), std::move(children)}) {}

  template<typename I>
  trie set(I begin, I end, V value) const {
    if (!_data)
      return trie().set(std::move(begin), std::move(end), std::move(value));

    if (begin == end)
      return trie(std::move(value), _data->children);

    auto map = _data->children;
    map[*begin] = std::move(map[*begin]).set(std::next(begin), end, std::move(value));
    return trie(_data->value, std::move(map));
  }

  template<typename R>
  trie set(const R &range, V value) const {
    return set(lib::begin(range), lib::end(range), std::move(value));
  }

  const V &get() const {
    static const V fallback;
    return _data ? _data->value : fallback;
  }

  trie child(K key) const {
    if (!_data)
      return *this;

    std::lock_guard<std::mutex> _(_data->mutex);
    return _data->children[key];
  }

  template<typename I>
  trie descendant(I begin, I end) const {
    return begin == end ? *this : child(*begin).descendant(std::next(begin), end);
  }

  template<typename R>
  trie descendant(const R &range) const {
    return descendant(lib::begin(range), lib::end(range));
  }

  template<typename I>
  const V &get(I begin, I end) const {
    return descendant(std::move(begin), std::move(end)).get();
  }

  template<typename R>
  const V &get(const R &range) const {
    return get(lib::begin(range), lib::end(range));
  }

  const std::unordered_map<K, trie> &children() const { return _data->children; }
};

}} // namespace halcheck::lib

#endif
