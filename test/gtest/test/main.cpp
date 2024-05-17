#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/lib/thread.hpp>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iterator>
#include <mutex>
#include <stdexcept>
#define DOCTEST_CONFIG_IMPLEMENT

#include <halcheck/ext/gtest.hpp>
#include <halcheck/fmt/show.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>

#include <future>
#include <thread>

using namespace halcheck;

TEST(gtest, example) {
  ::testing::GTEST_FLAG(throw_on_failure) = false;
  EXPECT_NONFATAL_FAILURE(
      halcheck::ext::gtest::check([] {
        auto xs = gen::arbitrary<std::vector<bool>>();
        EXPECT_LE(xs.size(), 10);
      }),
      "");
}

HALCHECK_TEST(gtest, skip) { GTEST_SKIP(); }

HALCHECK_TEST(gtest, binary_search, test::limit(test::random(), 1024)) {
  using limits = std::numeric_limits<int8_t>;
  auto min = gen::range(limits::min(), limits::max());
  auto max = int8_t(gen::range(min, limits::max()) + 1);
  auto err = gen::range(min, max);
  std::clog << "min: " << fmt::show(min) << std::endl;
  std::clog << "max: " << fmt::show(max) << std::endl;
  std::clog << "err: " << fmt::show(err) << std::endl;
  try {
    test::check([&] {
      auto x = gen::range(min, max);
      if (x >= err) {
        std::clog << "> x: " << fmt::show(x) << std::endl;
        throw x;
      }
    });
  } catch (const decltype(err) &e) {
    EXPECT_EQ(e, err);
  }
}

class counter {
public:
  void inc() {
    using namespace std::chrono_literals;

    std::uintmax_t x = value;
    if (x < 10)
      value = x + 1;
    else
      value = 0;
  }

  std::uintmax_t get() const { return value; }

private:
  std::mutex mutex;
  std::atomic_uintmax_t value{0};
};

HALCHECK_TEST(gtest, counter_monotonicity) {
  if (!getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  ::testing::GTEST_FLAG(throw_on_failure) = true;

  class monitor {
  public:
    void inc() {
      std::clog << "inc()\n";
      base.inc();
    }

    void get() {
      std::clog << "get(): ";
      auto output = base.get();
      std::clog << output << "\n";
      EXPECT_LE(last, output);
      last = output;
    }

  private:
    std::uintmax_t last = 0;
    counter base;
  } counter;

  gen::repeat([&] { gen::element<std::function<void()>>({[&] { counter.get(); }, [&] { counter.inc(); }})(); });
}

HALCHECK_TEST(gtest, counter_linearizability, test::random()) {
  if (!getenv("HALCHECK_NOSKIP"))
    GTEST_SKIP();

  // The maximum number of concurrently executing commands.
  // This particular bug seems easier to catch the fewer threads we use.
  static const std::size_t MAXCONCURRENCY = 2;

  // A verifier consists of a timestamp (version) and a function (eval) which
  // checks the result of some function call on the given system (counter).
  // While eval requires a system (counter) as an argument, this could have been
  // implemented with a model instead.
  struct verifier {
    lib::version version;
    std::function<bool(counter &)> eval;
  };

  // The monitor wraps the get and inc functions of the system (counter) and,
  // for each call to these functions, records a verifier that can be run later
  // to check for consistency.
  //
  // Each function takes an extra id argument, which is just used to impose
  // artificial ordering constraints.
  struct {
    verifier get(std::size_t id) {
      std::lock_guard<std::mutex> lock(mutexes[id]);
      auto version = versions[id] = versions[id].bump(id);

      auto actual = system.get();
      std::clog << fmt::to_string(version) + ": get() → " + fmt::to_string(actual) + "\n";
      return {version, [=](counter &system) { return system.get() == actual; }};
    }

    verifier inc(std::size_t id) {
      std::lock_guard<std::mutex> lock(mutexes[id]);
      auto version = versions[id] = versions[id].bump(id);

      system.inc();
      std::clog << fmt::to_string(version) + ": inc()\n";
      return {version, [](counter &system) {
                system.inc();
                return true;
              }};
    }

    verifier sync() {
      std::vector<std::unique_lock<std::mutex>> locks;
      lib::version version;
      for (std::size_t i = 0; i < MAXCONCURRENCY; i++) {
        locks.emplace_back(mutexes[i]);
        version |= versions[i];
      }
      for (std::size_t i = 0; i < MAXCONCURRENCY; i++)
        version = version.bump(i);
      for (std::size_t i = 0; i < MAXCONCURRENCY; i++)
        versions[i] = version;

      std::clog << fmt::to_string(version) + ": sync()\n";
      return {version, [](counter &) { return true; }};
    }

    std::array<std::mutex, MAXCONCURRENCY> mutexes;
    std::array<lib::version, MAXCONCURRENCY> versions;

    std::mutex verifiers_mutex;
    std::vector<verifier> verifiers;

    counter system;
  } monitor;

  // We generate at most 9 commands since 10! > 3 * 10⁶!
  auto commands = gen::container<std::vector<std::function<verifier()>>>(gen::range(1, 10), [&] {
    auto thread = gen::range<std::size_t>(0, MAXCONCURRENCY);
    return gen::element<std::function<verifier()>>(
        {[&monitor, thread] { return monitor.get(thread); },
         [&monitor, thread] { return monitor.inc(thread); },
         [&monitor] { return monitor.sync(); }});
  });

  // Executes each command in its own thread.
  std::vector<verifier> verifiers;
  lib::parallel(commands, std::back_inserter(verifiers));

  EXPECT_TRUE(lib::any_permutation(verifiers.begin(), verifiers.end(), [&] {
    for (std::size_t i = 0; i < verifiers.size(); i++) {
      for (std::size_t j = i + 1; j < verifiers.size(); j++) {
        if (verifiers[j].version < verifiers[i].version)
          return false;
      }
    }

    counter system;
    return std::all_of(verifiers.begin(), verifiers.end(), [&](verifier &f) { return f.eval(system); });
  }));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
