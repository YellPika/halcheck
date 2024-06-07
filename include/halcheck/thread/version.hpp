#ifndef HALCHECK_THREAD_VERSION_HPP
#define HALCHECK_THREAD_VERSION_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>

#include <future>
#include <map>
#include <thread>
#include <type_traits>
#include <vector>

namespace halcheck { namespace thread {

/**
 * @brief An implementation of Lamport vector clocks.
 */
class version {
public:
  /**
   * @brief Orders this version after another.
   * @param other The version to be ordered after.
   * @return This version.
   */
  version &operator|=(const version &other);

  /**
   * @brief Computes the least upper bound of this version and another.
   * @param other The other version to compute the bound of.
   * @return The least upper bound.
   */
  version operator|(version other) const;

  /**
   * @brief Determines if this version occurs before another.
   * @param other The other version to compare to.
   * @return true iff this version "happens before" the other.
   */
  bool operator<(const version &other) const;

  /**
   * @brief Determines if this version equals another.
   * @param other The other version to compare to.
   * @return true iff this version is identical to the other.
   */
  bool operator==(const version &other) const;

  /**
   * @brief Prints a version to a stream.
   * @param os The stream to print to.
   * @param value The version to print.
   * @return The given stream.
   */
  friend std::ostream &operator<<(std::ostream &os, const version &value);

  /**
   * @brief Increments and gets the current thread's version.
   * @return The version.
   */
  static version next();

  /**
   * @brief Order's the current thread's version after the given version.
   * @param other The version to be ordered after.
   */
  static void join(const version &other);

private:
  std::map<std::size_t, std::uintmax_t> counters;
};

}} // namespace halcheck::thread

#endif
