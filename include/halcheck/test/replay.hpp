#ifndef HALCHECK_TEST_REPLAY_HPP
#define HALCHECK_TEST_REPLAY_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>

#include <fstream>
#include <vector>

namespace halcheck { namespace test {

template<typename Strategy>
struct replay_t {
  template<typename F>
  void operator()(F func) {
    struct sample_t {
      std::size_t level;
      bool next;
      lib::optional<uintmax_t> shrink;
    };

    std::vector<sample_t> input;
    {
      std::ifstream is(filename, std::ios_base::in | std::ios_base::binary);
      while (is) {
        sample_t sample;
        is.read(reinterpret_cast<char *>(&sample.level), sizeof(sample.level));
        is.read(reinterpret_cast<char *>(&sample.next), sizeof(sample.next));

        std::uintmax_t shrink;
        is.read(reinterpret_cast<char *>(&shrink), sizeof(shrink));
        if (shrink > 0)
          sample.shrink = shrink - 1;
        else
          sample.shrink.reset();

        input.push_back(sample);
      }
    }

    lib::invoke(strategy, [&] {
      std::size_t level = 0;
      auto i = input.begin();

      auto _0 = gen::group.handle([&](bool open) {
        if (open)
          ++level;
        else {
          --level;

          while (i != input.end() && i->level > level)
            ++i;

          if (i != input.end() && i->level >= level)
            ++i;
        }
      });

      auto _1 = gen::next.handle([&](std::uintmax_t w0, std::uintmax_t w1) {
        while (i != input.end() && i->level > level)
          ++i;

        if (i == input.end() || i->level < level)
          return gen::next(w0, w1);
        else
          return (i++)->next;
      });

      auto _2 = gen::shrink.handle([&](std::uintmax_t size) {
        while (i != input.end() && i->level > level)
          ++i;

        if (i == input.end() || i->level < level)
          return gen::shrink(size);
        else
          return (i++)->shrink;
      });

      lib::invoke(func);
    });
  }

  Strategy strategy;
  std::string filename;
};

template<typename Strategy>
replay_t<Strategy> replay(Strategy strategy, std::string filename) {
  return {std::move(strategy), std::move(filename)};
}

}} // namespace halcheck::test
#endif
