#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <map>
#include <string>
#include <vector>

using namespace halcheck;

class store {
public:
  void step() { _time++; }

  void put(std::string key, std::string value) { _data[std::move(key)][_time] = std::move(value); }

  lib::optional<std::string> get(const std::string &key, std::size_t time) const {
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
  std::size_t _time = 0;
  std::map<std::string, std::map<std::size_t, std::string>> _data;
};

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
  gen::repeat("init"_s, [&](lib::atom id) { gen::retry(id, [&](lib::atom id) { return gen::one(id, step, put); }); });

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
  gen::repeat("modify"_s, [&](lib::atom id) {
    gen::retry(id, [&](lib::atom id) { return gen::one(id, step, put_other); });
  });

  // Step 5: check that get returns the same value we wrote
  LOG(INFO) << "[get]";
  LOG(INFO) << "get(" << testing::PrintToString(key) << ", " << time << ")";
  auto result = sys.get(key, time);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(*result, value);
}
