#include "halcheck/test/shrink.hpp"

#include "json.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/forward_shrinks.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/shrinks.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/string.hpp>
#include <halcheck/lib/trie.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <unordered_map>
#include <vector>

using namespace halcheck;
using json = nlohmann::json;

test::strategy test::shrink(std::uintmax_t repetitions) {
  return [=](const std::function<void()> &func) {
    bool non_default = false;
    lib::trie<lib::atom, lib::optional<std::uintmax_t>> input;
    if (auto input_str = test::read("input")) {
      if (auto input_json = lib::of_string<json>(*input_str)) {
        try {
          input = input_json->get<lib::trie<lib::atom, lib::optional<std::uintmax_t>>>();
          non_default = true;
        } catch (const json::parse_error &) {
        }
      }
    }

    auto result = gen::make_shrinks(input, func);
    test::write("input", json(input).dump());
    try {
      result.get();
      for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
        result = gen::make_shrinks(input, func);
        result.get();
      }
    } catch (const gen::result &) {
      throw;
    } catch (...) {
      auto it = result.children().begin();
      while (it != result.children().end()) {
        auto next = gen::make_shrinks(*it, func);
        test::write("input", json(*it).dump());
        try {
          next.get();
          for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
            next = gen::make_shrinks(*it, func);
            next.get();
          }
        } catch (const gen::result &) {
        } catch (...) {
          input = *it;
          result = std::move(next);
          it = result.children().begin();
          continue;
        }

        ++it;
        test::write("input", json(input).dump());
      }

      result.get();
    }
  };
}

test::strategy test::forward_shrink(std::uintmax_t repetitions) {
  return [=](const std::function<void()> &func) {
    bool non_default = false;
    std::vector<uintmax_t> input;
    if (auto input_str = test::read("forward_input")) {
      if (auto input_json = lib::of_string<json>(*input_str)) {
        try {
          input = input_json->get<std::vector<std::uintmax_t>>();
          non_default = true;
        } catch (const json::parse_error &) {
        }
      }
    }

    auto result = gen::make_forward_shrinks(input, func);
    test::write("forward_input", json(input).dump());
    try {
      result.get();
      for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
        result = gen::make_forward_shrinks(input, func);
        result.get();
      }
    } catch (const gen::result &) {
      throw;
    } catch (...) {
      auto it = result.children().begin();
      while (it != result.children().end()) {
        auto next = gen::make_forward_shrinks(*it, func);
        test::write("forward_input", json(*it).dump());
        try {
          next.get();
          for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
            next = gen::make_forward_shrinks(*it, func);
            next.get();
          }
        } catch (const gen::result &) {
        } catch (...) {
          input = *it;
          result = std::move(next);
          it = result.children().begin();
          continue;
        }

        ++it;
        test::write("input", json(input).dump());
      }

      result.get();
    }
  };
}
