#include "halcheck/test/shrink.hpp"

#include "json.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/shrinks.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
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
    auto input = lib::trie<lib::atom, lib::optional<std::uintmax_t>>();
    if (auto input_str = test::read("input")) {
      if (auto input_json = lib::of_string<json>(*input_str)) {
        try {
          input = input_json->get<lib::trie<lib::atom, lib::optional<std::uintmax_t>>>();
          non_default = true;
        } catch (const json::parse_error &) {
        }
      }
    }

    auto result = gen::shrinks(input, func);
    test::write("input", json(input).dump());
    try {
      result.get();
      for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
        result = gen::shrinks(input, func);
        result.get();
      }
    } catch (const gen::result &) {
      throw;
    } catch (...) {
      auto it = result.children().begin();
      while (it != result.children().end()) {
        auto next = gen::shrinks(*it, func);
        test::write("input", json(*it).dump());
        try {
          next.get();
          for (std::uintmax_t i = 1; i < (non_default ? repetitions : 1); i++) {
            next = gen::shrinks(*it, func);
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
    // TODO: test-case reproduction

    struct handler : eff::handler<handler, gen::shrink_effect> {
      lib::optional<std::uintmax_t> operator()(gen::shrink_effect args) override {
        auto _ = lib::finally([&] { index++; });

        while (index >= sizes.size())
          sizes.push_back(0);
        sizes[index] = args.size;

        if (index < input.size() && input[index] && args.size > 0)
          return std::min(*input[index], args.size - 1);
        else
          return lib::nullopt;
      }

      std::size_t index = 0;
      std::vector<lib::optional<std::uintmax_t>> input;
      std::vector<std::uintmax_t> sizes;
    } handler;

    try {
      eff::handle(func, handler);
    } catch (const gen::result &) {
      throw;
    } catch (...) {
      auto e = std::current_exception();

      auto &index = handler.index;
      auto &sizes = handler.sizes;
      auto &input = handler.input;

      index = 0;
      while (index < sizes.size()) {
        // Ensure we've allocated enough input to work with the current index.
        while (index >= input.size())
          input.emplace_back();

        // Move to next index if current index can't be shrunk.
        if (sizes[index] == 0) {
          index++;
          continue;
        }

        if (!input[index]) // Produce first shrink haven't shrunk before
          input[index] = 0;
        else if (*input[index] < sizes[index]) // Produce next shrink if we haven't reached the last
          ++*input[index];
        else { // Move to next index if current index can't be shrunk further.
          index++;
          continue;
        }

        try {
          for (std::uintmax_t i = 0; i < repetitions; i++)
            eff::handle(func, handler);
        } catch (const gen::result &) {
        } catch (...) {
          index++;
          e = std::current_exception();
        }
      }

      std::rethrow_exception(e);
    }
  };
}
