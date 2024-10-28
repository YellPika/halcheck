#ifndef JSON_HPP
#define JSON_HPP

#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/trie.hpp>
#include <halcheck/lib/variant.hpp>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>     // IWYU pragma: export

#include <unordered_map>
#include <utility>

namespace halcheck { namespace lib {

inline void to_json(nlohmann::json &j, const lib::atom &t) {
  lib::visit(
      lib::make_overload(
          [&](lib::number value) { j = (lib::number::value_type)value; },
          [&](lib::symbol value) { j = (lib::symbol::value_type)value; }),
      t);
}

inline void from_json(const nlohmann::json &j, lib::atom &t) {
  if (j.is_string())
    t = lib::symbol(j.get<lib::symbol::value_type>());
  else
    t = lib::number(j.get<lib::number::value_type>());
}

}} // namespace halcheck::lib

namespace nlohmann {
template<typename K, typename V, typename Hash>
struct adl_serializer<halcheck::lib::trie<K, V, Hash>> {
  static void to_json(nlohmann::json &j, const halcheck::lib::trie<K, V, Hash> &t) {
    using namespace halcheck;
    nlohmann::json::object_t object;
    object["value"] = *t;
    object["children"] = std::unordered_map<K, lib::trie<K, V, Hash>, Hash>(t.begin(), t.end());
    j = std::move(object);
  }

  static void from_json(const nlohmann::json &j, halcheck::lib::trie<K, V, Hash> &t) {
    auto value = j.at("value").get<V>();
    auto children = j.at("children").get<std::unordered_map<K, halcheck::lib::trie<K, V>>>();
    t = halcheck::lib::trie<K, V>(std::move(value), std::move(children));
  }
};

template<typename T>
struct adl_serializer<halcheck::lib::optional<T>> {
  static void to_json(nlohmann::json &j, const halcheck::lib::optional<T> &opt) {
    if (opt == halcheck::lib::nullopt) {
      j = nullptr;
    } else {
      j = *opt;
    }
  }

  static void from_json(const nlohmann::json &j, halcheck::lib::optional<T> &opt) {
    if (j.is_null()) {
      opt = halcheck::lib::nullopt;
    } else {
      opt = j.template get<T>();
    }
  }
};
} // namespace nlohmann

#endif
