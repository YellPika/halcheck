#ifndef HALCHECK_TEST_REPLAY_HPP
#define HALCHECK_TEST_REPLAY_HPP

#include <halcheck/fmt/log.hpp>
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
public:
  template<typename F>
  void operator()(F func) {
    struct sample_t {
      std::size_t level;
      bool next;
      lib::optional<uintmax_t> shrink;
    };

    auto directory = this->directory.value_or(default_directory());
    if (directory.empty()) {
      lib::invoke(strategy, std::move(func));
      return;
    }

    std::vector<sample_t> input;
    {
      std::ifstream is(directory + "/" + filename, std::ios_base::in | std::ios_base::binary);
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

      if (!input.empty())
        fmt::log(fmt::test_case_replay{directory + "/" + filename});
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
  lib::optional<std::string> directory;

private:
  static std::string default_directory() {
    static const char *var1 = std::getenv("HALCHECK_REPLAY");
    static const char *var2 = std::getenv("HALCHECK_RECORD");
    static std::string dir = var1 ? std::string(var1) : var2 ? std::string(var2) : ".";
    return dir;
  }
};

/// @brief Constructs a strategy where inputs are drawn from a given file.
/// @tparam Strategy The type of base strategy.
/// @param strategy A base strategy used if the file doesn't exist or doesn't contain enough data.
/// @param filename The input file to read.
/// @param directory The directory containing the file to read. The default value is chosen as follows:
///                  1. $HALCHECK_REPLAY, if the environment variable HALCHECK_REPLAY is set.
///                  2. $HALCHECK_RECORD, if the environment variable HALCHECK_RECORD is set.
///                  3. The current working directory.
///                  If the empty string is specified, then this strategy behaves identically to the base strategy.
/// @remark The input file is read entirely before a test starts. Changes to the file during test-case execution will
///         not affect the operation of this strategy.
/// @return A strategy where inputs are drawn from a given file.
template<typename Strategy>
replay_t<Strategy> replay(Strategy strategy, std::string filename, lib::optional<std::string> directory = {}) {
  return {std::move(strategy), std::move(filename), std::move(directory)};
}

}} // namespace halcheck::test
#endif
