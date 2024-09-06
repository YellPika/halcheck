#include "halcheck/test/deserialize.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/test/serialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <ghc/filesystem.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
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

struct handler : eff::handler<handler, test::read_effect, test::write_effect> {
  explicit handler(entry config) : config(std::move(config)) {}

  lib::optional<std::string> operator()(test::read_effect args) override {
    auto it = config.data.find(args.key);
    if (it != config.data.end())
      return it->second;
    else
      return lib::nullopt;
  }

  void operator()(test::write_effect args) override {
    config.data[args.key] = args.value;
    std::ofstream(config.filename, std::ios::trunc) << json(config.data);
  }

  entry config;
};

struct strategy {
  void operator()(const std::function<void()> &func) {
    for (auto &&config : entries) {
      try {
        eff::handle(func, handler(config));
      } catch (const gen::result &) {
        // NOLINT: no error
      }
    }
  }

  std::vector<entry> entries;
};
} // namespace

test::strategy test::deserialize(std::string name, std::string folder) {
  auto directory = fs::path(std::move(folder)) / std::move(name);
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

  return ::strategy{std::move(entries)};
}