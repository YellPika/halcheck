#include "halcheck/test/deserialize.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/optional.hpp>
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

test::strategy test::deserialize(std::string name, std::string folder) {
  namespace fs = ghc::filesystem;
  using json = nlohmann::json;
  using pair = std::pair<fs::file_time_type, std::unordered_map<std::string, std::string>>;

  auto directory = fs::path(std::move(folder)) / std::move(name);
  fs::create_directories(directory);

  std::vector<pair> saved;
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
    saved.emplace_back(file.last_write_time(), value.get<std::unordered_map<std::string, std::string>>());
  }

  std::sort(saved.begin(), saved.end(), [](const pair &lhs, const pair &rhs) { return lhs.first > rhs.first; });

  struct handler : eff::handler<handler, test::read_effect> {
    explicit handler(std::unordered_map<std::string, std::string> config) : config(std::move(config)) {}

    lib::optional<std::string> operator()(test::read_effect args) override {
      auto it = config.find(args.key);
      if (it != config.end())
        return it->second;
      else
        return lib::nullopt;
    }

    std::unordered_map<std::string, std::string> config;
  };

  struct {
    void operator()(const std::function<void()> &func) {
      for (auto &&config : saved) {
        try {
          eff::handle(func, handler(config.second));
        } catch (const gen::result &) {
          // NOLINT: no error
        }
      }
    }

    std::vector<pair> saved;
  } output{std::move(saved)};

  return output;
}
