#include "halcheck/test/deserialize.hpp"

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <ghc/filesystem.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace halcheck;
namespace fs = ghc::filesystem;
using json = nlohmann::json;

namespace {
struct entry {
  fs::file_time_type time;
  std::unordered_map<std::string, std::string> data;
  std::string filename;

  bool operator<(const entry &other) const { return time > other.time; }
};

struct handler : lib::effect::handler<handler, test::read_effect, test::write_effect, gen::succeed_effect> {
  explicit handler(entry config) : config(std::move(config)) {}

  lib::optional<std::string> operator()(test::read_effect args) {
    auto it = config.data.find(args.key);
    if (it != config.data.end())
      return it->second;
    else
      return test::read(std::move(args.key));
  }

  void operator()(test::write_effect args) {
    config.data[args.key] = std::move(args.value);
    std::ofstream(config.filename, std::ios::trunc) << json(config.data);
  }

  void operator()(gen::succeed_effect) {}

  entry config;
};

struct strategy {
  void operator()(lib::function_view<void()> func) const {
    auto folder = test::read("FOLDER").value_or(".halcheck");
    auto directory = fs::path(std::move(folder)) / name;
    fs::create_directories(directory);

    std::vector<entry> entries;
    for (auto &&file : fs::directory_iterator(directory)) {
      if (!file.is_regular_file())
        continue;

      json value;
      try {
        std::ifstream(file.path()) >> value;
      } catch (const json::parse_error &) {
        continue;
      }

      if (!value.is_object())
        continue;

      entries.push_back(
          entry{file.last_write_time(), value.get<std::unordered_map<std::string, std::string>>(), file.path()});
    }

    std::sort(entries.begin(), entries.end());

    for (auto &&config : entries) {
      try {
        handler(config).handle(func);
      } catch (const gen::result_exception &) {
        // NOLINT: no error
      }
    }
  }

  std::string name;
};
} // namespace

test::strategy test::deserialize(std::string name) { return ::strategy{std::move(name)}; }
