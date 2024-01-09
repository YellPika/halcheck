#ifndef HALCHECK_TEST_SHRINK_HPP
#define HALCHECK_TEST_SHRINK_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/strategy.hpp>

#include <algorithm>
#include <cstddef>
#include <exception>
#include <vector>

namespace halcheck { namespace test {

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
class shrink_t {
public:
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F>())>
  void operator()(F func) const {
    std::vector<sample> samples;
    std::vector<shrink> shrinks;
    try {
      lib::invoke(strategy, [&] {
        samples.clear();
        shrinks.clear();
        exec(func, samples, shrinks);
      });
    } catch (...) {
      struct discard {};
      auto e = std::current_exception();

      for (std::size_t i = 0; i < shrinks.size();) {
        bool success = false;
        for (std::uintmax_t j = 0; j < shrinks[i].size; j++) {
          auto csamples = samples;
          csamples[shrinks[i].index].shrink = j;
          std::vector<shrink> cshrinks;
          try {
            auto _0 = gen::succeed.handle([] {});
            auto _1 = gen::discard.handle([] { return discard(); });
            auto _2 = gen::next.handle([](std::uintmax_t w0, std::uintmax_t w1) { return w0 == 0 && w1 > 0; });
            exec(func, csamples, cshrinks);
          } catch (const discard &) {
          } catch (...) {
            e = std::current_exception();
            samples = std::move(csamples);
            shrinks = std::move(cshrinks);
            success = true;
            break;
          }
        }

        i = success ? 0 : i + 1;
      }

      std::rethrow_exception(e);
    }
  }

  Strategy strategy;

private:
  struct sample {
    std::uintmax_t level;
    bool value;
    lib::optional<std::uintmax_t> shrink;
  };

  struct shrink {
    std::size_t index;
    std::uintmax_t size;
  };

  template<typename F>
  void exec(F func, std::vector<sample> &samples, std::vector<shrink> &shrinks) const {
    std::size_t level = 0;
    auto i = samples.begin();

    auto _ = lib::finally([&] {
      std::stable_sort(shrinks.begin(), shrinks.end(), [&](const shrink &lhs, const shrink &rhs) {
        return samples[lhs.index].level < samples[rhs.index].level;
      });
    });

    auto _1 = gen::group.handle([&](bool open) {
      if (open)
        ++level;
      else {
        --level;

        while (i != samples.end() && i->level > level)
          ++i;

        if (i == samples.end() || i->level < level)
          i = samples.insert(i, sample{level, false, lib::nullopt});

        ++i;
      }
    });

    auto _2 = gen::next.handle([&](const gen::weight &w0, const gen::weight &w1) {
      while (i != samples.end() && i->level > level)
        ++i;

      if (i == samples.end() || i->level < level)
        i = samples.insert(i, sample{level, gen::next(w0, w1), lib::nullopt});

      return (i++)->value;
    });

    auto _3 = gen::shrink.handle([&](std::uintmax_t size) -> lib::optional<std::uintmax_t> {
      while (i != samples.end() && i->level > level)
        ++i;

      if (i == samples.end() || i->level < level)
        i = samples.insert(i, sample{level, false, gen::shrink(size)});

      auto j = i++;
      auto output = j->shrink;

      if (size > 0 && !output)
        shrinks.push_back({std::size_t(std::distance(samples.begin(), j)), size});

      if (size == 0 || !output)
        return lib::nullopt;
      else
        return std::min(*output, size - 1);
    });

    lib::invoke(func);
  }
};

template<typename Strategy, HALCHECK_REQUIRE(test::is_strategy<Strategy>())>
constexpr shrink_t<Strategy> shrink(Strategy strategy) {
  return {std::move(strategy)};
}

}} // namespace halcheck::test

#endif