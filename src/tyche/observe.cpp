#include "halcheck/tyche/observe.hpp"

#include "nlohmann/json.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/strategy.hpp>

#include <ghc/filesystem.hpp>

#include <chrono>
#include <exception>
#include <fstream>
#include <functional>
#include <ostream>
#include <random>
#include <string>
#include <utility>

using json = nlohmann::json;
namespace fs = ghc::filesystem;
using namespace halcheck;

namespace {
struct strategy {
  strategy(std::string name, std::string directory)
      : name(name), os(std::move(directory) + "/" + std::move(name) + ".jsonl"),
        run_start(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                      .count()) {}

  void operator()(lib::function_view<void()> func) const {
    json object;
    object["type"] = "test_case";
    object["status"] = "passed";
    object["status_reason"] = "";
    object["representation"] = "";
    object["arguments"] = json::object();
    object["how_generated"] = "";
    object["features"] = json::object();
    object["coverage"] = nullptr;
    object["timing"] = json::object();
    object["metadata"] = json::object();
    object["property"] = name;
    object["run_start"] = run_start;
    auto start = std::chrono::high_resolution_clock::now();
    auto _ = lib::finally([&] {
      auto end = std::chrono::high_resolution_clock::now();
      object["timing"]["execute:test"] = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
      os << object << "\n";
    });

    try {
      func();
    } catch (const gen::discard &) {
      object["status"] = "gave_up";
      throw;
    } catch (const std::exception &e) {
      object["status_reason"] = e.what();
      object["status"] = "failed";
      throw;
    } catch (...) {
      object["status"] = "failed";
      throw;
    }
  }

  std::string name;
  mutable std::ofstream os;
  long run_start;
};
} // namespace

test::strategy tyche::observe(std::string name, std::string folder) { // NOLINT
  fs::create_directories(folder);
  return test::make_strategy<::strategy>(std::move(name), std::move(folder));
}
