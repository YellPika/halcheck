#include <algorithm>
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

HALCHECK_TEST(gtest, linearizability, test::random()) {
  using namespace std::chrono_literals;

  std::atomic_int counter = 0;

  auto inc = [=, &counter] {
    int x = counter;
    std::this_thread::sleep_for(10ms);
    counter = x + 1;
    std::clog << "inc: " << x << " → " << x + 1 << "\n";
    return [=, &counter] {
      counter++;
      return true;
    };
  };

  auto get = [=, &counter] {
    int x = counter;
    std::clog << "get: " << x << "\n";
    return [=, &counter] {
      int y = counter;
      return x == y;
    };
  };

  using checker = std::function<bool()>;
  using command = std::function<checker()>;
  using stamp = std::map<std::size_t, std::size_t>;

  auto actions = gen::container<std::vector<command>>(gen::range(1, 9), [&] {
    return gen::element<command>({inc, get});
  });

  std::vector<std::future<std::pair<stamp, checker>>> tasks;
  std::vector<std::mutex> locks(gen::range(0, 3));
  std::vector<std::size_t> clock(locks.size());
  for (auto &&action : actions) {
    auto deps = gen::container<std::set<std::size_t>>([&] { return gen::range(0, locks.size()); });
    tasks.push_back(std::async(std::launch::async, [&, deps, action] {
      stamp stamp;
      for (auto i : deps) {
        locks[i].lock();
        stamp[i] = ++clock[i];
      }
      auto _ = lib::finally([&] {
        for (auto i : deps)
          locks[i].unlock();
      });
      return std::make_pair(stamp, action());
    }));
  }

  std::vector<std::tuple<std::size_t, stamp, checker>> checkers;
  for (auto &&task : tasks) {
    auto pair = task.get();
    checkers.push_back({checkers.size(), pair.first, pair.second});
  }

  do {
    std::fill(clock.begin(), clock.end(), 0);

    bool ok = true;
    for (auto &&checker : checkers) {
      for (auto &&pair : std::get<1>(checker)) {
        if (pair.second > clock[pair.first])
          clock[pair.first] = pair.second;
        else {
          ok = false;
          break;
        }
      }

      if (!ok)
        break;
    }

    if (!ok)
      continue;

    counter = 0;
    if (std::all_of(checkers.begin(), checkers.end(), [](const std::tuple<std::size_t, stamp, checker> &func) {
          return std::get<2>(func)();
        }))
      return;
  } while (std::next_permutation(
      checkers.begin(),
      checkers.end(),
      [](const std::tuple<std::size_t, stamp, checker> &lhs, const std::tuple<std::size_t, stamp, checker> &rhs) {
        return std::get<0>(lhs) < std::get<0>(rhs);
      }));

  FAIL();
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
