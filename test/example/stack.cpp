#include "utility.hpp"

#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include "gtest/gtest.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <ostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace halcheck;
using namespace halcheck::lib::literals;

class stack {
public:
  stack() : _top(nullptr) {}
  stack(stack &&) = delete;
  stack &operator=(stack &&) = delete;
  stack(const stack &) = delete;
  stack &operator=(const stack &) = delete;

  ~stack() {
    while (_top)
      delete _top.exchange(_top.load()->next);
  }

  void push(int value) {
    auto top = new node{value, _top.load()};
    delay();
    while (!_top.compare_exchange_strong(top->next, top))
      delay();
  }

  int pop() {
    node *top;
    do {
      top = _top.load();
      if (!top)
        throw std::runtime_error("empty stack");
      delay();
    } while (!_top.compare_exchange_strong(top, top->next));
    auto _ = lib::finally([&] { delete top; });
    return top->value;
  }

private:
  struct node {
    int value;
    node *next;
  };

  std::atomic<node *> _top;
};

HALCHECK_TEST(Stack, Model) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  struct monitor {
    void push() {
      auto value = gen::arbitrary<int>("value"_s);
      LOG(INFO) << "push(" << value << ")";
      object.push(value);
      model.push_back(value);
    }

    void pop() {
      gen::guard(!model.empty());

      auto expected = model.back();
      auto _ = lib::finally([&] { model.pop_back(); });

      auto actual = object.pop();
      LOG(INFO) << "pop -> " << actual;
      EXPECT_EQ(expected, actual);
    }

    stack object;
    std::vector<int> model;
  } m;

  gen::repeat("commands"_s, [&](lib::atom id) {
    gen::retry(id, [&](lib::atom id) {
      auto _ = gen::label(id);
      auto command = gen::noshrink([] { return gen::element("command"_s, &monitor::push, &monitor::pop); });
      gen::label("exec"_s, command, m);
    });
  });
}

HALCHECK_TEST(Stack, Linearizability) {
  using namespace lib::literals;

  // Example test; skip by default
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  stack object;          // The system under test
  std::vector<bool> set; // Indicates which threads have a value on the stack

  // The serializability monitor is parameterized by two types:
  // - std::size_t: The type of thread IDs.
  // - std::vector<int>: The model being tested against.
  lib::serializability_monitor<std::size_t, std::vector<int>> monitor;

  // The push command
  auto push = [&](lib::atom id) {
    auto _ = gen::label(id);

    // Chooses a random value to push
    auto value = gen::arbitrary<int>("value"_s);

    // Chooses random threads to run on
    // Initially this will just be one thread
    // During shrinking this may shrink to the set of "all threads",
    // which effectively serializes this command
    auto threads = gen_threads("threads"_s);

    // We also use an extra thread corresponding to a new value
    // This is used to ensure pop only runs on a non-empty stack
    threads.push_back(set.size() + max_threads);
    set.push_back(true);

    // Finally, we submit the command to the monitor
    monitor.invoke(threads, [=, &object] {
      LOG(INFO) << "[system] push(" << value << ") on threads " << testing::PrintToString(threads);
      object.push(value);

      // System commands return the corresponding model command
      // (In this case, a function which pushes the value into a vector.)
      return [=](std::vector<int> &model) { model.push_back(value); };
    });
  };

  auto pop = [&](lib::atom id) {
    auto _ = gen::label(id);

    auto threads = gen_threads("threads"_s);

    gen::retry("index"_s, [&](lib::atom id) {
      auto index = gen::range(id, 0, set.size());
      gen::guard(set[index]);
      threads.push_back(index + max_threads);
      set[index] = false;
    });

    monitor.invoke(threads, [=, &object] {
      LOG(INFO) << "[system] pop() on threads " << testing::PrintToString(threads);
      auto actual = object.pop();
      LOG(INFO) << " -> " << actual;
      return [=](std::vector<int> &model) {
        EXPECT_GT(model.size(), 0);
        auto _ = lib::finally([&] { model.pop_back(); });
        EXPECT_EQ(actual, model.back());
      };
    });
  };

  gen::repeat("commands"_s, [&](lib::atom id) {
    gen::retry(id, [&](lib::atom id) { return gen::one(id, push, pop); });
  });

  EXPECT_TRUE(monitor.check());
}
