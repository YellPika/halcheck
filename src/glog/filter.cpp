#include "halcheck/glog/filter.hpp"

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/strategy.hpp>

#include <glog/logging.h>

#include <cstddef>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>

using namespace halcheck;

namespace {
struct sink : google::LogSink {
  void send(
      google::LogSeverity severity,
      const char *,
      const char *filename,
      int line,
      const google::LogMessageTime &time,
      const char *message,
      std::size_t size) override {
    const std::lock_guard<std::mutex> lock(mutex);
    output += ToString(severity, filename, line, time, message, size) + "\n";
  }

  std::mutex mutex;
  std::string output;
};

struct strategy {
  void operator()(lib::function_view<void()> func) const {
    std::string output;
    std::size_t iteration = 0;
    std::size_t succeeded = 0;
    std::size_t discarded = 0;
    std::size_t shrinks = 0;
    bool failed = false;

    auto _ = lib::finally([&] {
      if (failed) {
        std::clog << "\nFailed after " << succeeded << " test(s), " << discarded << " discard(s), and " << shrinks
                  << " shrink(s)\n\n";
        if (!output.empty())
          std::clog << "Log:\n" << output << "\n";
      } else {
        std::clog << "\nSucceeded after " << succeeded << " test(s) and " << discarded << " discard(s)\n";
      }
    });

    inner([&] {
      auto i = iteration++;
      sink sink;
      try {
        {
          google::AddLogSink(&sink);
          auto _ = lib::finally([&] { google::RemoveLogSink(&sink); });
          func();
        }
        if (!failed)
          succeeded++;

        LOG(INFO) << "Test Case (" << i << "): SUCCESS";
      } catch (const gen::discard_exception &) {
        LOG(INFO) << "Test Case (" << i << "): DISCARD";
        discarded++;
        throw;
      } catch (const gen::result_exception &) {
        // TODO: handle succeed_effect and check if it was really called.
        LOG(INFO) << "Test Case (" << i << "): COMPLETE";
        throw;
      } catch (...) {
        if (failed)
          shrinks++;
        else
          failed = true;

        LOG(INFO) << "Test Case (" << i << "): FAILURE";
        output = std::move(sink.output);
        failed = true;
        throw;
      }
    });
  }

  test::strategy inner;
};
} // namespace

test::strategy glog::filter(test::strategy inner) { return strategy{std::move(inner)}; }
