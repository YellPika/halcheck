#ifndef JSON_HPP
#define JSON_HPP

#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/trie.hpp>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp> // IWYU pragma: export

namespace halcheck { namespace lib {

template<typename K, typename V>
void to_json(nlohmann::json &j, const lib::trie<K, V> &t) {
  j = nlohmann::json{
      {"value",    t.get()     },
      {"children", t.children()}
  };
}

template<typename K, typename V>
void from_json(const nlohmann::json &j, lib::trie<K, V> &t) {
  auto value = j.at("value").get<V>();
  auto children = j.at("children").get<std::unordered_map<K, lib::trie<K, V>>>();
  t = lib::trie<K, V>(std::move(value), std::move(children));
}

void to_json(nlohmann::json &j, const lib::atom &t) {
  lib::visit(lib::overload([&](lib::number value) { j = *value; }, [&](lib::symbol value) { j = *value; }), t);
}

void from_json(const nlohmann::json &j, lib::atom &t) {
  if (j.is_string())
    t = lib::symbol(j.get<std::string>());
  else
    t = lib::number(j.get<std::uintmax_t>());
}

}} // namespace halcheck::lib

namespace nlohmann {
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
