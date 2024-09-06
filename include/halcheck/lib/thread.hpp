#ifndef HALCHECK_LIB_THREAD_HPP
#define HALCHECK_LIB_THREAD_HPP

#include <halcheck/lib/dag.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <future>
#include <unordered_map>
#include <vector>

namespace halcheck { namespace lib {

template<
    typename ThreadID,
    typename State,
    HALCHECK_REQUIRE(lib::is_hashable<ThreadID>()),
    HALCHECK_REQUIRE(lib::is_copyable<State>())>
struct serializability_monitor {
public:
  explicit serializability_monitor(State seed = State()) : _seed(std::move(seed)) {}

  template<
      typename R,
      typename F,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<std::function<void(State &)>, F>())>
  void invoke(R range, F func);

  template<typename F, HALCHECK_REQUIRE(lib::is_invocable_r<std::function<void(State &)>, F>())>
  void invoke(const std::initializer_list<ThreadID> &range, F func) {
    return invoke(range.begin(), range.end(), std::move(func));
  }

  template<
      typename I,
      typename F,
      HALCHECK_REQUIRE(lib::is_iterator<I>()),
      HALCHECK_REQUIRE(lib::is_invocable_r<std::function<void(State &)>, F>())>
  void invoke(I begin, I end, F func) {
    return invoke(lib::make_view(std::move(begin), std::move(end)), std::move(func));
  }

  bool check();

private:
  using future = std::shared_future<std::function<void(State &)>>;
  using iterator = lib::iterator_t<lib::dag<future>>;
  using const_iterator = lib::const_iterator_t<lib::dag<future>>;

  bool check(State, std::vector<std::size_t> &, std::vector<const_iterator> &) const;

  std::unordered_map<ThreadID, iterator> _threads;
  lib::dag<future> _transitions;
  State _seed;
};

template<
    typename ThreadID,
    typename State,
    HALCHECK_REQUIRE_(lib::is_hashable<ThreadID>()) C1,
    HALCHECK_REQUIRE_(lib::is_copyable<State>()) C2>
template<
    typename R,
    typename F,
    HALCHECK_REQUIRE_(lib::is_range<R>()),
    HALCHECK_REQUIRE_(lib::is_invocable_r<std::function<void(State &)>, F>())>
void lib::serializability_monitor<ThreadID, State, C1, C2>::invoke(R range, F func) {
  std::vector<iterator> parent_iterators;
  for (auto &&res : range) {
    auto it = _threads.find(res);
    if (it != _threads.end())
      parent_iterators.push_back(it->second);
  }

  std::vector<future> parent_futures;
  parent_futures.reserve(parent_iterators.size());
  for (auto &&i : parent_iterators)
    parent_futures.push_back(*i);

  auto it = _transitions.emplace(
      std::move(parent_iterators),
      std::async(
          std::launch::async,
          [](F func, const std::vector<future> &parents) -> std::function<void(State &)> {
            for (auto &&parent : parents)
              parent.wait();

            return lib::invoke(func);
          },
          std::move(func),
          std::move(parent_futures))
          .share());

  for (auto &&res : range)
    _threads[res] = it;
}

template<
    typename ThreadID,
    typename State,
    HALCHECK_REQUIRE_(lib::is_hashable<ThreadID>()) C1,
    HALCHECK_REQUIRE_(lib::is_copyable<State>()) C2>
bool lib::serializability_monitor<ThreadID, State, C1, C2>::check() {
  std::vector<std::size_t> references(_transitions.size(), 0);
  for (auto i = _transitions.begin(); i != _transitions.end(); ++i) {
    (*i).get();
    for (auto j : _transitions.children(i))
      ++references[j - _transitions.begin()];
  }

  std::vector<const_iterator> queue(_transitions.roots().begin(), _transitions.roots().end());
  return check(_seed, references, queue);
}

template<
    typename ThreadID,
    typename State,
    HALCHECK_REQUIRE_(lib::is_hashable<ThreadID>()) C1,
    HALCHECK_REQUIRE_(lib::is_copyable<State>()) C2>
bool lib::serializability_monitor<ThreadID, State, C1, C2>::check(
    State seed, std::vector<std::size_t> &references, std::vector<const_iterator> &queue) const {
  for (std::size_t i = 0; i < queue.size(); ++i) {
    auto next = seed;
    auto it = queue[i];
    try {
      lib::invoke((*it).get(), next);
    } catch (...) {
      continue;
    }

    for (auto j : _transitions.children(it)) {
      if (--references[j - _transitions.begin()] == 0)
        queue.push_back(j);
    }

    std::swap(queue[i], queue.back());
    queue.pop_back();

    auto _ = lib::finally([&] {
      queue.push_back(it);
      std::swap(queue[i], queue.back());

      for (auto j : _transitions.children(it)) {
        if (references[j - _transitions.begin()]++ == 0)
          queue.pop_back();
      }
    });

    if (check(next, references, queue))
      return true;
  }

  return queue.empty();
}

}} // namespace halcheck::lib

#endif
