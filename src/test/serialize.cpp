#include "halcheck/test/serialize.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/strategy.hpp>

#include <ghc/filesystem.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iterator>
#include <random>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>

using namespace halcheck;
namespace fs = ghc::filesystem;
using json = nlohmann::json;

namespace {
struct handler : eff::handler<handler, test::write_effect> {
  handler(std::unordered_map<std::string, std::string> config, std::string filename)
      : config(std::move(config)), filename(std::move(filename)) {}

  void operator()(test::write_effect args) override {
    config[args.key] = args.value;
    std::ofstream(filename, std::ios::trunc) << json(config);
  }

  std::unordered_map<std::string, std::string> config;
  std::string filename;
};

struct strategy {
  void operator()(std::function<void()> func) {
    std::error_code code;
    fs::rename(filename, filename + ".bak", code);

    bool success = false;
    auto _ = lib::finally([&] {
      if (!success)
        return;

      if (code)
        fs::remove(filename);
      else
        fs::rename(filename + ".bak", filename, code);
    });

    try {
      eff::handle(std::move(func), handler({}, filename));
      success = true;
    } catch (const gen::result &) {
      success = true;
      throw;
    }
  }

  std::string filename;
};
} // namespace

test::strategy test::serialize(std::string name, std::string folder) {
  static const std::string table = "0123456789ABCDEF";

  auto directory = fs::path(std::move(folder)) / std::move(name);
  fs::create_directories(directory);

  std::string id;
  std::random_device device;
  std::uniform_int_distribution<std::size_t> dist(0, table.size() - 1);
  std::generate_n(std::back_inserter(id), table.size(), [&] { return table[dist(device)]; });
  auto filename = directory / id;

  return ::strategy{std::move(filename)};
}
