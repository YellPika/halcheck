#include "halcheck/test/config.hpp"

#include <halcheck/eff/api.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/test/deserialize.hpp>
#include <halcheck/test/strategy.hpp>

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <utility>

using namespace halcheck;

test::strategy test::config(const std::initializer_list<test::set> &config) {
  struct strategy {
    struct handler : eff::handler<handler, test::read_effect> {
      explicit handler(std::unordered_map<std::string, std::string> config) : config(std::move(config)) {}

      lib::optional<std::string> operator()(test::read_effect args) override {
        auto it = config.find(args.key);
        if (it != config.end())
          return it->second;
        else
          return test::read(std::move(args.key));
      }

      std::unordered_map<std::string, std::string> config;
    };

    void operator()(lib::function_view<void()> func) const { eff::handle(func, handler(config)); }

    std::unordered_map<std::string, std::string> config;
  };

  std::unordered_map<std::string, std::string> map;
  for (auto &&pair : config)
    map[pair.key] = pair.value;
  return strategy{std::move(map)};
}
