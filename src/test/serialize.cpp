#include "halcheck/test/serialize.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <ghc/filesystem.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <ios>
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
  void operator()(lib::function_view<void()> func) const {
    static const std::string table = "0123456789ABCDEF";

    auto folder = test::read("FOLDER").value_or(".halcheck");
    auto directory = fs::path(std::move(folder)) / name;
    fs::create_directories(directory);

    std::string id;
    std::random_device device;
    std::uniform_int_distribution<std::size_t> dist(0, table.size() - 1);
    std::generate_n(std::back_inserter(id), table.size(), [&] { return table[dist(device)]; });
    std::string filename = directory / id;

    std::error_code code;
    fs::rename(filename, filename + ".bak", code);

    eff::handle(std::move(func), handler({}, filename));

    if (code)
      fs::remove(filename);
    else
      fs::rename(filename + ".bak", filename, code);
  }

  std::string name;
};
} // namespace

test::strategy test::serialize(std::string name) { return ::strategy{std::move(name)}; }
