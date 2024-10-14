#ifndef JSON_HPP
#define JSON_HPP

#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/trie.hpp>
#include <halcheck/lib/variant.hpp>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp> // IWYU pragma: export

#include <string>
#include <unordered_map>

namespace halcheck { namespace lib {

void from_json(const nlohmann::json &j, lib::symbol &s) { s = lib::symbol(j.get<std::string>()); }

void from_json(const nlohmann::json &j, lib::number &n) { n = lib::number(j.get<lib::number::value_type>()); }

void to_json(nlohmann::json &j, const lib::symbol &s) { j = (const std::string &)s; }

void to_json(nlohmann::json &j, const lib::number &n) { j = (const lib::number::value_type &)n; }

template<typename K, typename V>
void to_json(nlohmann::json &j, const lib::trie<K, V> &t) {
  j = nlohmann::json{
      {"value",    *t          },
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
  lib::visit(lib::make_overload([&](lib::number value) { j = value; }, [&](lib::symbol value) { j = value; }), t);
}

void from_json(const nlohmann::json &j, lib::atom &t) {
  if (j.is_string())
    t = j.get<lib::symbol>();
  else
    t = j.get<lib::number>();
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
