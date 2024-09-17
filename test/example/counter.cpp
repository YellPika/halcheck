#include "utility.hpp"

#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace halcheck;
using namespace halcheck::lib::literals;

class counter {
public:
  counter() = default;
  counter(counter &&other) noexcept : value(other.value.load()) {}
  counter(const counter &other) : value(other.value.load()) {}
  counter &operator=(const counter &other) {
    if (this != &other)
      value = other.value.load();
    return *this;
  }
  counter &operator=(counter &&other) noexcept {
    if (this != &other)
      value = other.value.load();
    return *this;
  }
  ~counter() = default;

  void inc() {
    static thread_local std::random_device random;

    int i = value.load();
    delay();
    value.store(i + 1 < 10 ? i + 1 : 0);
  }

  int get() const { return value; }

private:
  std::atomic_int value{0};
};

HALCHECK_TEST(Counter, Model) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  counter object;
  int previous = -1;

  auto inc = [&](lib::atom) {
    LOG(INFO) << "[system] inc()";
    object.inc();
  };

  auto get = [&](lib::atom) {
    LOG(INFO) << "[system] get()";
    auto current = object.get();
    LOG(INFO) << "  -> " << current;
    ASSERT_GE(current, previous);
    previous = current;
  };

  gen::repeat("commands"_s, [&](lib::atom id) { gen::one(id, inc, get); });
}

HALCHECK_TEST(Counter, Linearizability) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  counter object;
  lib::serializability_monitor<int, counter> monitor;

  auto get = [&](lib::atom id) {
    auto threads = gen_threads(id);
    LOG(INFO) << "[monitor] get() on threads " << testing::PrintToString(threads);
    monitor.invoke(threads, [=, &object] {
      auto actual = object.get();
      LOG(INFO) << "[system] get(): " << actual << " on threads " << testing::PrintToString(threads);
      return [=](const counter &expected) { EXPECT_EQ(actual, expected.get()); };
    });
  };

  auto inc = [&](lib::atom id) {
    auto threads = gen_threads(id);
    LOG(INFO) << "[monitor] inc() on threads " << testing::PrintToString(threads);
    monitor.invoke(threads, [=, &object] {
      LOG(INFO) << "[system] inc() on threads " << testing::PrintToString(threads);
      object.inc();
      return [=](counter &model) { model.inc(); };
    });
  };

  gen::repeat("commands"_s, [&](lib::atom id) { gen::one(id, get, inc); });

  EXPECT_TRUE(monitor.check());
}
