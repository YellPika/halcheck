#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <halcheck.hpp>

#include <chrono>
#include <random>
#include <thread>

static void delay() {
  static thread_local std::random_device random;
  std::this_thread::sleep_for(std::chrono::microseconds(std::uniform_int_distribution<int>(0, 50)(random)));
}

static const std::size_t max_threads = 2;

static std::vector<std::size_t> gen_threads() {
  using namespace halcheck;
  using namespace lib::literals;

  std::vector<std::size_t> output(max_threads);
  std::iota(output.begin(), output.end(), 0);

  if (!gen::shrink("seq"_s))
    output = {gen::element_of("thread"_s, output)};

  return output;
}

#endif
