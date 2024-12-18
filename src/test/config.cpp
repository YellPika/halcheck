#include "halcheck/test/config.hpp"

#include <halcheck/lib/effect.hpp>
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
  struct entry {
    std::string value;
    bool weak;
  };

  struct strategy {
    struct handler : lib::effect::handler<handler, test::read_effect> {
      explicit handler(std::unordered_map<std::string, entry> config) : config(std::move(config)) {}

      lib::optional<std::string> operator()(test::read_effect args) final {
        auto it = config.find(args.key);
        if (it == config.end())
          return test::read(std::move(args.key));

        if (it->second.weak) {
          if (auto value = test::read(std::move(args.key)))
            return value;
        }

        return it->second.value;
      }

      std::unordered_map<std::string, entry> config;
    };

    void operator()(lib::function_view<void()> func) const { handler(config).handle(func); }

    std::unordered_map<std::string, entry> config;
  };

  std::unordered_map<std::string, entry> map;
  for (auto &&pair : config)
    map[pair.key] = entry{pair.value, pair.weak};
  return strategy{std::move(map)};
}
