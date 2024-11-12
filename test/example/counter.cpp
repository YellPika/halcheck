#include "utility.hpp"

#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>
#include <halcheck/lib/functional.hpp>

#include <atomic>
#include <ostream>
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
    const int i = value.load();
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

  for (auto _ : gen::repeat("commands"_s))
    gen::one("command"_s, inc, get);
}

HALCHECK_TEST(Counter, Linearizability) {
  if (!lib::getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  struct get_command {};
  struct inc_command {};
  using command = lib::variant<get_command, inc_command>;

  auto dag = gen::dag("dag"_s, [](lib::atom id) {
    return gen::one(
        id,
        [](lib::atom id) -> std::pair<std::vector<std::size_t>, command> {
          auto _ = gen::label(id);
          auto threads = gen_threads("threads"_s);
          LOG(INFO) << "[gen] get @ " << testing::PrintToString(threads);
          return std::make_pair(threads, get_command());
        },
        [](lib::atom id) -> std::pair<std::vector<std::size_t>, command> {
          auto _ = gen::label(id);
          auto threads = gen_threads("threads"_s);
          LOG(INFO) << "[gen] inc @ " << testing::PrintToString(threads);
          return std::make_pair(threads, inc_command());
        });
  });

  counter object;
  auto results = lib::async(dag, [&](lib::dag<command>::const_iterator it) {
    return lib::visit(
        lib::make_overload(
            [&](get_command) -> lib::move_only_function<bool(counter &) const> {
              auto expected = object.get();
              LOG(INFO) << "[async] get(): " << expected;
              return [=](counter &model) { return model.get() == expected; };
            },
            [&](inc_command) -> lib::move_only_function<bool(counter &) const> {
              object.inc();
              LOG(INFO) << "[async] inc()";
              return [](counter &model) {
                model.inc();
                return true;
              };
            }),
        *it);
  });

  counter model;
  EXPECT_TRUE(lib::linearize(results, model));
}
