#include "utility.hpp"

#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <atomic>
#include <cstddef>
#include <ostream>
#include <stdexcept>
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
    while (!_top.compare_exchange_strong(top->next, top)) {
      delay();
      top->next = _top.load();
    }
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

  for (auto _ : gen::repeat("commands"_s)) {
    gen::retry("command"_s, [&](lib::atom id) {
      auto _ = gen::label(id);
      auto command = gen::noshrink([] { return gen::element("command"_s, &monitor::push, &monitor::pop); });
      gen::label("exec"_s, command, m);
    });
  }
}

HALCHECK_TEST(Stack, Consistency) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  stack object;

  auto modify = [&](lib::atom id) {
    std::size_t size = 0;
    for (auto _ : gen::repeat(id)) {
      gen::retry("command"_s, [&](lib::atom id) {
        gen::one(
            id,
            [&](lib::atom id) {
              auto _ = gen::label(id);
              ++size;
              object.push(gen::arbitrary<int>("value"_s));
            },
            [&](lib::atom id) {
              auto _ = gen::label(id);
              gen::guard(size > 0);
              --size;
              object.pop();
            });
      });
    }
    return size;
  };

  modify("init"_s);

  auto value = gen::arbitrary<int>("value"_s);
  object.push(value);

  for (auto i = modify("modify"_s); i > 0; --i)
    object.pop();

  EXPECT_EQ(object.pop(), value);
}

HALCHECK_TEST(Stack, Linearizability) {
  using namespace lib::literals;

  // Example test; skip by default
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  struct push_command {
    int value;
  };
  struct pop_command {};
  using command = lib::variant<push_command, pop_command>;

  std::size_t counter = 0;
  std::set<std::size_t> set;
  auto dag = gen::schedule("dag"_s, [&](lib::atom id) {
    return gen::retry(id, [&](lib::atom id) {
      return gen::one(
          id,
          [&](lib::atom id) -> std::pair<std::vector<std::size_t>, command> {
            auto _ = gen::label(id);
            auto threads = gen_threads("threads"_s);
            threads.push_back(counter + max_threads);
            set.insert(counter++);
            LOG(INFO) << "[gen] push @ " << testing::PrintToString(threads);
            return std::make_pair(threads, push_command{gen::arbitrary<int>("value"_s)});
          },
          [&](lib::atom id) -> std::pair<std::vector<std::size_t>, command> {
            auto _ = gen::label(id);
            gen::guard(!set.empty());
            auto threads = gen_threads("threads"_s);
            auto index = gen::element_of("index"_s, set);
            set.erase(index);
            threads.push_back(index + max_threads);
            LOG(INFO) << "[gen] pop @ " << testing::PrintToString(threads);
            return std::make_pair(threads, pop_command());
          });
    });
  });

  struct push_result {
    int value;
  };
  struct pop_result {
    int value;
  };
  using result = lib::variant<push_result, pop_result>;

  stack object;
  auto results = lib::async(dag, [&](lib::dag<command>::const_iterator it) {
    return lib::visit(
        lib::make_overload(
            [&](push_command push) -> result {
              object.push(push.value);
              LOG(INFO) << "[async] push(" << push.value << ")";
              return push_result{push.value};
            },
            [&](pop_command) -> result {
              auto value = object.pop();
              LOG(INFO) << "[async] pop(): " << value;
              return pop_result{value};
            }),
        *it);
  });

  std::vector<int> model;
  EXPECT_TRUE(lib::linearize(results, model, [](const result &result, std::vector<int> &model) -> bool {
    return lib::visit(
        lib::make_overload(
            [&](push_result push) -> bool {
              model.push_back(push.value);
              return true;
            },
            [&](pop_result pop) -> bool {
              auto value = model.back();
              model.pop_back();
              return value == pop.value;
            }),
        result);
  }));
}
