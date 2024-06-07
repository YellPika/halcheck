#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/thread.hpp>
#include <halcheck/thread/mutex.hpp>

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
  std::uintmax_t inc() {
    std::uintmax_t output = value;
    if (output < 10)
      value = output + 1;
    else
      value = 0;
    return output;
  }

  void reset() { value = 0; }

private:
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
      auto output = system.inc();
      EXPECT_GE(output, last);
      last = output;
    }

    void reset() {
      std::clog << "reset()\n";
      last = 0;
      system.reset();
    }

  private:
    std::uintmax_t last = 0;
    counter system;
  } counter;

  gen::repeat([&] {
    gen::weighted<std::function<void()>>({
        {2, [&] { counter.inc(); }  },
        {1, [&] { counter.reset(); }}
    })();
  });
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
    thread::version version;
    std::function<bool(std::uintmax_t &)> eval;

    bool operator<(const verifier &other) const { return version < other.version; }
  };

  // The monitor wraps the get and inc functions of the system (counter) and,
  // for each call to these functions, records a verifier that can be run later
  // to check for consistency.
  //
  // Each function takes an extra id argument, which is just used to impose
  // artificial ordering constraints.
  struct {
    verifier inc(std::size_t id) {
      std::lock_guard<thread::mutex> lock(mutexes[id]);

      auto actual = system.inc();

      auto version = thread::version::next();
      std::clog << fmt::to_string(version) + ": inc() → " << actual << "\n";
      return {version, [actual](std::uintmax_t &model) {
                auto output = model == actual;
                if (model == 10)
                  model = 0;
                else
                  ++model;
                return output;
              }};
    }

    verifier reset() {
      std::vector<std::unique_lock<thread::mutex>> locks;
      for (auto &&mutex : mutexes)
        locks.emplace_back(mutex);

      system.reset();

      auto version = thread::version::next();
      std::clog << fmt::to_string(version) + ": reset()\n";
      return {version, [](std::uintmax_t &model) {
                model = 0;
                return true;
              }};
    }

    std::array<thread::mutex, MAXCONCURRENCY> mutexes;

    counter system;
  } monitor;

  // We generate at most 9 commands since 10! > 3 * 10⁶!
  auto commands = gen::container<std::vector<std::function<verifier()>>>(gen::range(1, 10), [&] {
    auto thread = gen::range<std::size_t>(0, MAXCONCURRENCY);
    return gen::element<std::function<verifier()>>(
        {[&monitor, thread] { return monitor.inc(thread); }, [&monitor] { return monitor.reset(); }});
  });

  // Executes each command in its own thread.
  std::vector<verifier> verifiers;
  thread::parallel(commands, std::back_inserter(verifiers));

  auto result = lib::any_sorted(verifiers.begin(), verifiers.end(), [&] {
    std::uintmax_t model = 0;
    return std::all_of(verifiers.begin(), verifiers.end(), [&](verifier &f) { return f.eval(model); });
  });

  EXPECT_TRUE(result);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
