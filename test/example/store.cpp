#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>
#include <halcheck/lib/functional.hpp>

#include <cstddef>
#include <map>
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace halcheck;

class store {
public:
  void step() {
    std::lock_guard<std::mutex> lock(_mutex);
    _time++;
  }

  void put(std::string key, std::string value) {
    std::lock_guard<std::mutex> lock(_mutex);
    _data[std::move(key)][_time] = std::move(value);
  }

  lib::optional<std::string> get(const std::string &key, std::size_t time) const {
    std::lock_guard<std::mutex> lock(_mutex);

    auto i = _data.find(key);
    if (i == _data.end())
      return lib::nullopt;

    auto &inner = i->second;

    // Incorrect version:
    auto j = inner.lower_bound(time);
    if (j == inner.end()) {
      if (j == inner.begin())
        return lib::nullopt;
      else {
        --j;
        return j->second;
      }
    } else
      return j->second;

    // Correct version:
    // auto j = inner.upper_bound(time);
    // if (j == inner.begin())
    //   return lib::nullopt;

    // --j;
    // return j->second;
  }

private:
  mutable std::mutex _mutex;
  std::size_t _time = 0;
  std::map<std::string, std::map<std::size_t, std::string>> _data;
};

HALCHECK_TEST(Store, Concurrency) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  using namespace lib::literals;

  // Step 0 (Types): we have a command type which may either be a get command or
  // a put command. The get command records the expected output.
  struct get {
    std::string key;
    lib::optional<std::string> value;
  };

  struct put {
    std::string key;
    std::string value;
  };

  using command = lib::variant<get, put>;

  store sys;
  std::map<std::string, lib::optional<std::string>> model;

  // Step 1: generate a small, finite set of keys. We only generate commands
  // that read/write to these keys to increase the chance of reading/writing to
  // the same value.
  auto keys = gen::noshrink([] {
    auto size = gen::range("size"_s, 1, 10);
    auto output = gen::container<std::set<std::string>>("keys"_s, size, gen::arbitrary<std::string>);
    return std::vector<std::string>(output.begin(), output.end());
  });

  // Step 2: generate a directed acyclic graph of commands.
  auto dag = gen::dag<std::string>("dag"_s, [&](lib::atom id, lib::function_view<void(std::string)> use) -> command {
    auto _ = gen::label(id);

    // We generate a random key to read/write. This command is ordered after the
    // last command that used this key, i.e. two commands run in parallel iff
    // they operate on different keys.
    auto key = gen::element_of("key"_s, keys);
    use(key);

    return gen::variant(
        "command"_s,
        [&](lib::atom id) {
          auto _ = gen::label(id);
          return get{key, model[key]};
        },
        [&](lib::atom id) {
          auto _ = gen::label(id);
          model[key] = gen::arbitrary<std::string>("value"_s);
          return put{key, *model[key]};
        });
  });

  // Step 3: execute the commands in parallel.
  lib::async(dag, [&](const command &cmd) {
    lib::visit(
        lib::make_overload(
            [&](const get &cmd) { EXPECT_EQ(sys.get(cmd.key, 0), cmd.value); },
            [&](const put &cmd) { sys.put(cmd.key, cmd.value); }),
        cmd);
    return 0;
  });
}

HALCHECK_TEST(Store, Consistency) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  using namespace lib::literals;

  auto keys = gen::noshrink([] {
    auto size = gen::range("size"_s, 1, 10);
    auto output = gen::container<std::set<std::string>>("keys"_s, size, gen::arbitrary<std::string>);
    return std::vector<std::string>(output.begin(), output.end());
  });

  store sys;
  std::size_t now = 0;

  auto step = [&](lib::atom) {
    LOG(INFO) << "step";
    sys.step();
    ++now;
  };

  auto put = [&](lib::atom id) {
    auto _ = gen::label(id);
    auto key = gen::range("key"_s, keys.begin(), keys.end());
    auto value = gen::arbitrary<std::string>("value"_s);
    LOG(INFO) << "put(" << testing::PrintToString(*key) << ", " << testing::PrintToString(value) << ")";
    sys.put(*key, value);
  };

  // Step 1: obtain a random state by performing random commands
  LOG(INFO) << "[init]";
  for (auto _ : gen::repeat("init"_s))
    gen::retry("command"_s, [&](lib::atom id) { return gen::one(id, step, put); });

  // Step 2: put a random key-value pair
  LOG(INFO) << "[put]";
  auto key = gen::element_of("key"_s, keys);
  auto value = gen::arbitrary<std::string>("value"_s);
  LOG(INFO) << "put(" << testing::PrintToString(key) << ", " << testing::PrintToString(value) << ")";
  sys.put(key, value);

  // Step 3: pick a random time at some point in the future
  // (we test consistency relative to this point)
  auto time = now + gen::range("time"_s, 0, gen::size() + 1);

  auto put_other = [&](lib::atom id) {
    auto _ = gen::label(id);
    auto index = gen::range("key"_s, keys.begin(), keys.end());
    gen::guard(*index != key || now > time);
    auto value = gen::arbitrary<std::string>("value"_s);
    LOG(INFO) << "put(" << testing::PrintToString(*index) << ", " << testing::PrintToString(value) << ")";
    sys.put(*index, value);
  };

  // Step 4: perform a random set of commands that DO NOT affect the value of
  // assinged to key `key' at time `time'.
  LOG(INFO) << "[modify]";
  for (auto _ : gen::repeat("modify"_s))
    gen::retry("command"_s, [&](lib::atom id) { return gen::one(id, step, put_other); });

  // Step 5: check that get returns the same value we wrote
  LOG(INFO) << "[get]";
  LOG(INFO) << "get(" << testing::PrintToString(key) << ", " << time << ")";
  auto result = sys.get(key, time);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(*result, value);
}
