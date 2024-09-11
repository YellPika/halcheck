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

  auto inc = [&] {
    LOG(INFO) << "[system] inc()";
    object.inc();
  };

  auto get = [&] {
    LOG(INFO) << "[system] get()";
    auto current = object.get();
    LOG(INFO) << "  -> " << current;
    ASSERT_GE(current, previous);
    previous = current;
  };

  gen::repeat([&] { gen::one(inc, get); });
}

HALCHECK_TEST(Counter, Linearizability) {
  using namespace lib::literals;

  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  counter object;
  lib::serializability_monitor<int, int> monitor{0};

  auto get = [&] {
    auto threads = gen_threads();
    monitor.invoke(threads, [=, &object] {
      auto actual = object.get();
      LOG(INFO) << "[system] get(): " << actual << " on threads " << testing::PrintToString(threads);
      return [=](int expected) { EXPECT_EQ(actual, expected); };
    });
  };

  auto inc = [&] {
    auto threads = gen_threads();
    monitor.invoke(threads, [=, &object] {
      LOG(INFO) << "[system] inc() on threads " << testing::PrintToString(threads);
      object.inc();
      return [=](int &model) { ++model; };
    });
  };

  gen::repeat([&] { gen::one(get, inc); });

  EXPECT_TRUE(monitor.check());
}
