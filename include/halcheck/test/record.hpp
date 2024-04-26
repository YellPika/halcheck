#ifndef HALCHECK_TEST_RECORD_HPP
#define HALCHECK_TEST_RECORD_HPP

#include <halcheck/fmt/log.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/scope.hpp>

#include <fstream>

namespace halcheck { namespace test {

template<typename Strategy>
struct record_t {
  template<typename F>
  void operator()(F func) {
    struct sample_t {
      std::size_t level;
      bool next;
      lib::optional<uintmax_t> shrink;

      std::ostream &write(std::ostream &os) const {
        os.write(reinterpret_cast<const char *>(&level), sizeof(level));
        os.write(reinterpret_cast<const char *>(&next), sizeof(next));

        auto flat = shrink ? *shrink + 1 : 0;
        os.write(reinterpret_cast<char *>(&flat), sizeof(flat));
        return os;
      }
    };

    if (!directory) {
      lib::invoke(strategy, std::move(func));
      return;
    }

    auto path = *directory + "/" + filename;
    fmt::log(fmt::test_case_record{path});

    auto input = [&] {
      std::ifstream is(path, std::ios_base::in | std::ios_base::binary);
      return std::string(std::istreambuf_iterator<char>(is), {});
    }();

    lib::invoke(strategy, [&] {
      bool success = false;
      auto _ = lib::finally([&] {
        if (success) {
          if (input.empty())
            std::remove(path.c_str());
          else {
            std::ofstream os(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
            os << input;
          }
        } else {
          std::ifstream is(path, std::ios_base::in | std::ios_base::binary);
          input = std::string(std::istreambuf_iterator<char>(is), {});
        }
      });

      std::size_t level = 0;
      std::ofstream os(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

      auto _0 = gen::group.handle([&](bool open) {
        if (open)
          ++level;
        else {
          --level;
          sample_t{level, false, lib::nullopt}.write(os);
        }
      });

      auto _1 = gen::next.handle([&](std::uintmax_t w0, std::uintmax_t w1) {
        auto output = gen::next(w0, w1);
        sample_t{level, output, lib::nullopt}.write(os);
        return output;
      });

      auto _2 = gen::shrink.handle([&](std::uintmax_t size) {
        auto output = gen::shrink(size);
        sample_t{level, false, output}.write(os);
        return output;
      });

      auto _3 = gen::discard.handle([&] {
        success = true;
        return gen::discard();
      });

      auto _4 = gen::succeed.handle([&] {
        success = true;
        gen::succeed();
        success = false;
      });

      lib::invoke(func);
      success = true;
    });
  }

  Strategy strategy;
  std::string filename;
  lib::optional<std::string> directory;
};

lib::optional<std::string> default_record_directory();

template<typename Strategy>
record_t<Strategy>
record(Strategy strategy, std::string filename, lib::optional<std::string> directory = default_record_directory()) {
  return {std::move(strategy), std::move(filename), std::move(directory)};
}

}} // namespace halcheck::test
#endif
