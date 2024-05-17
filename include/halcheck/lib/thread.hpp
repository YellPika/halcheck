#ifndef HALCHECK_LIB_THREAD_HPP
#define HALCHECK_LIB_THREAD_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>

#include <future>
#include <map>
#include <thread>
#include <type_traits>
#include <vector>

namespace halcheck { namespace lib {

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
  for (auto i = begin; i != end; i++) {
    threads.push_back(std::async(std::launch::async, [i, &future] {
      future.wait();
      return lib::invoke(*i);
    }));
  }
  promise.set_value();

  for (auto &&thread : threads) {
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
  for (auto i = begin; i != end; i++) {
    threads.push_back(std::async(std::launch::async, [i, &future] {
      future.wait();
      lib::invoke(*i);
    }));
  }
  promise.set_value();

  for (auto &&thread : threads) {
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

/**
 * @brief An implementation of Lamport vector clocks.
 */
class version {
public:
  /**
   * @brief Registers a new event for the specified node.
   * @param id The id of the node to register an event for.
   * @returns The incremented version.
   */
  version bump(std::size_t id) const;

  /**
   * @brief Computes the least upper bound of this version and another.
   * @param other The other version to compute the bound of.
   * @returns The least upper bound.
   */
  version &operator|=(const version &other);

  /**
   * @brief Computes the least upper bound of this version and another.
   * @param other The other version to compute the bound of.
   * @returns The least upper bound.
   */
  version operator|(version other) const;

  /**
   * @brief Determines if this version occurs before another.
   * @param other The other version to compare to.
   * @returns true iff this version "happens before" the other.
   */
  bool operator<(const version &other) const;

  /**
   * @brief Determines if this version equals another.
   * @param other The other version to compare to.
   * @returns true iff this version is identical to the other.
   */
  bool operator==(const version &other) const;

  /**
   * @brief Prints a version to a stream.
   * @param os The stream to print to.
   * @param value The version to print.
   * @returns The given stream.
   */
  friend std::ostream &operator<<(std::ostream &os, const version &value);

private:
  std::map<std::size_t, std::uintmax_t> counters;
};

}} // namespace halcheck::lib

#endif
