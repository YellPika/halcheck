#ifndef HALCHECK_LIB_THREAD_HPP
#define HALCHECK_LIB_THREAD_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/ranges.hpp>

#include <future>
#include <map>
#include <set>
#include <type_traits>
#include <vector>

namespace halcheck { namespace lib {

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

class version {
public:
  void bump(std::size_t i);
  version &operator|=(const version &other);
  bool operator<(const version &other) const;
  bool operator==(const version &other) const;
  friend std::ostream &operator<<(std::ostream &os, const version &value);

private:
  std::map<std::size_t, std::uintmax_t> counters;
};

class pool {
public:
  class resource {
  public:
    class version version;

  private:
    friend class pool;
    std::vector<std::unique_lock<std::mutex>> locks;
  };

  explicit pool(std::size_t size) : resources(size) {}

  std::size_t size() const { return resources.size(); }

  resource lock(const std::initializer_list<std::size_t> &range) { return lock(range.begin(), range.end()); }

  template<
      typename T,
      HALCHECK_REQUIRE(lib::is_range<T>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<T>, std::size_t>())>
  resource lock(const T &range) {
    return lock(lib::begin(range), lib::end(range));
  }

  template<
      typename I,
      HALCHECK_REQUIRE(lib::is_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<typename std::iterator_traits<I>::reference, std::size_t>())>
  resource lock(I begin, I end) {
    std::set<std::size_t> indices(begin, end);

    resource output;
    for (auto &&i : indices) {
      auto &resource = resources[i];
      std::unique_lock<std::mutex> lock(resource.mutex);
      output.locks.push_back(std::move(lock));
      output.version |= resource.version;
      output.version.bump(i);
    }

    for (auto &&i : indices)
      resources[i].version = output.version;

    return output;
  }

private:
  struct data {
    class version version;
    std::mutex mutex;
  };

  std::vector<data> resources;
};

}} // namespace halcheck::lib

#endif
