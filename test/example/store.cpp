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

HALCHECK_TEST(Store, KeyCompositionality) {
  using namespace lib::literals;

  auto keys = gen::noshrink("keys"_s, [] {
    auto output = gen::container<std::set<std::string>>(gen::range(1, 10), gen::arbitrary);
    return std::vector<std::string>(output.begin(), output.end());
  });

  store sys;
  std::vector<store> models(keys.size());
  std::size_t now = 0;

  auto step = [&] {
    LOG(INFO) << "step";
    sys.step();
    for (auto &model : models)
      model.step();
    ++now;
  };

  auto put = [&] {
    auto key = gen::range("key"_s, keys.begin(), keys.end());
    std::string value = gen::arbitrary("value"_s);
    LOG(INFO) << "put(" << *key << ", " << value << ")";

    sys.put(*key, value);
    models[key - keys.begin()].put("", value);
  };

  auto get = [&] {
    auto key = gen::range("key"_s, keys.begin(), keys.end());
    auto time = gen::range("time"_s, 0, now + 2);
    LOG(INFO) << "get(" << *key << ", " << time << ")";
    auto actual = sys.get(*key, time);
    auto expected = models[key - keys.begin()].get("", time);
    EXPECT_EQ(actual, expected);
  };

  gen::repeat("commands"_s, [&] { gen::one(step, put, get); });
}

HALCHECK_TEST(Store, Consistency) {
  using namespace lib::literals;

  auto keys = gen::noshrink("keys"_s, [] {
    auto output = gen::container<std::set<std::string>>(gen::range(1, 10), gen::arbitrary);
    return std::vector<std::string>(output.begin(), output.end());
  });

  store sys;
  std::size_t now = 0;

  auto step = [&] {
    LOG(INFO) << "step";
    sys.step();
    ++now;
  };

  auto put = [&] {
    auto key = gen::range("key"_s, keys.begin(), keys.end());
    std::string value = gen::arbitrary("value"_s);
    LOG(INFO) << "put(" << testing::PrintToString(*key) << ", " << testing::PrintToString(value) << ")";
    sys.put(*key, value);
  };

  LOG(INFO) << "[init]";
  gen::repeat("init"_s, [&] { gen::retry(gen::one, step, put); });

  LOG(INFO) << "[put]";
  auto key = gen::element_of("key"_s, keys);
  std::string value = gen::arbitrary("value"_s);
  auto time = now + gen::range("time"_s, 0, gen::size() + 1);
  LOG(INFO) << "put(" << testing::PrintToString(key) << ", " << testing::PrintToString(value) << ")";
  sys.put(key, value);

  auto put_other = [&] {
    auto index = gen::range("key"_s, keys.begin(), keys.end());
    gen::guard(*index != key || now > time);
    std::string value = gen::arbitrary("value"_s);
    LOG(INFO) << "put(" << testing::PrintToString(*index) << ", " << testing::PrintToString(value) << ")";
    sys.put(*index, value);
  };

  LOG(INFO) << "[modify]";
  gen::repeat("modify"_s, [&] { gen::retry(gen::one, step, put_other); });

  LOG(INFO) << "[get]";
  LOG(INFO) << "get(" << testing::PrintToString(key) << ", " << time << ")";
  auto result = sys.get(key, time);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(*result, value);
}
