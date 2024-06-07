#ifndef HALCHECK_THREAD_PARALLEL_HPP
#define HALCHECK_THREAD_PARALLEL_HPP

#include <halcheck/lib/ranges.hpp>
#include <halcheck/thread/version.hpp>

#include <future>
#include <vector>

namespace halcheck { namespace thread {

/**
 * @brief Executes a range of functions in parallel.
 * @tparam I The type of iterator pointing to the functions to execute.
 * @tparam O The type of iterator in which to store the result.
 * @param begin The iterator pointing to the first function to execute.
 * @param end Indicates the end of the range of functions.
 * @param output The iterator into which the results of each function are stored.
 */
template<
    typename I,
    typename O,
    HALCHECK_REQUIRE(lib::is_iterator<I>()),
    HALCHECK_REQUIRE(lib::is_invocable<typename std::iterator_traits<I>::reference>()),
    HALCHECK_REQUIRE(lib::is_iterator<O>()),
    HALCHECK_REQUIRE(std::is_assignable<
                     decltype(*std::declval<O>()),
                     lib::invoke_result_t<typename std::iterator_traits<I>::reference>>())>
void parallel(I begin, I end, O output) {
  using T = decltype(lib::invoke(*begin));

  std::vector<std::future<T>> threads;
  std::promise<void> promise;
  auto future = promise.get_future();
  auto version = thread::version::next();
  for (auto i = begin; i != end; i++) {
    threads.push_back(std::async(std::launch::async, [i, &future, &version] {
      thread::version::join(version);
      future.wait();
      return lib::invoke(*i);
    }));
  }
  promise.set_value();

  for (auto &&thread : threads) {
    thread::version::next();
    try {
      *output++ = std::move(thread.get());
    } catch (...) {
      for (auto &&thread : threads) {
        try {
          thread.wait();
        } catch (...) {
        }
      }
      throw;
    }
  }
}

/**
 * @brief Executes a range of functions in parallel.
 * @tparam I The type of iterator pointing to the functions to execute.
 * @param begin The iterator pointing to the first function to execute.
 * @param end Indicates the end of the range of functions.
 */
template<
    typename I,
    HALCHECK_REQUIRE(lib::is_iterator<I>()),
    HALCHECK_REQUIRE(lib::is_invocable<typename std::iterator_traits<I>::reference>())>
void parallel(I begin, I end) {
  using T = decltype(lib::invoke(*begin));

  std::vector<std::future<T>> threads;
  std::promise<void> promise;
  auto future = promise.get_future();
  auto version = thread::version::next();
  for (auto i = begin; i != end; i++) {
    threads.push_back(std::async(std::launch::async, [i, &future, &version] {
      thread::version::join(version);
      future.wait();
      lib::invoke(*i);
    }));
  }
  promise.set_value();

  for (auto &&thread : threads) {
    thread::version::next();
    try {
      thread.wait();
    } catch (...) {
      for (auto &&thread : threads) {
        try {
          thread.wait();
        } catch (...) {
        }
      }
      throw;
    }
  }
}

/**
 * @brief Executes a range of functions in parallel.
 * @tparam T The type of range of functions.
 * @tparam O The type of iterator in which to store the result.
 * @param range The range of functions.
 * @param output The iterator into which the results of each function are stored.
 */
template<
    typename T,
    typename O,
    HALCHECK_REQUIRE(lib::is_range<T>()),
    HALCHECK_REQUIRE(lib::is_invocable<lib::range_reference_t<T>>()),
    HALCHECK_REQUIRE(lib::is_iterator<O>()),
    HALCHECK_REQUIRE(
        std::is_assignable<decltype(*std::declval<O>()), lib::invoke_result_t<lib::range_reference_t<T>>>())>
void parallel(const T &range, O output) {
  parallel(lib::begin(range), lib::end(range), std::move(output));
}

/**
 * @brief Executes a range of functions in parallel.
 * @tparam T The type of range of functions.
 * @param range The range of functions.
 */
template<
    typename T,
    HALCHECK_REQUIRE(lib::is_range<T>()),
    HALCHECK_REQUIRE(lib::is_invocable<lib::range_reference_t<T>>())>
void parallel(const T &range) {
  parallel(lib::begin(range), lib::end(range));
}

}} // namespace halcheck::thread

#endif
