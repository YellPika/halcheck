#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <halcheck.hpp>
#include <halcheck/gen/arbitrary.hpp>

#include <chrono>
#include <random>
#include <thread>

static inline void delay() {
  static thread_local std::random_device random;
  std::this_thread::sleep_for(std::chrono::microseconds(std::uniform_int_distribution<int>(0, 50)(random)));
}

static const std::size_t max_threads = 2;

static inline std::vector<std::size_t> gen_threads(halcheck::lib::atom id) {
  using namespace halcheck;
  using namespace lib::literals;

  auto _ = gen::label(id);

  std::vector<std::size_t> output;

  auto keep = gen::noshrink(gen::range, "keep"_s, 0, max_threads);
  for (std::size_t i = 0; i < max_threads; i++) {
    if (i == keep || !gen::arbitrary<bool>(i))
      output.push_back(i);
  }

  return output;
}

#endif
