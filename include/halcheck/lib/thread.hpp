#ifndef HALCHECK_LIB_THREAD_HPP
#define HALCHECK_LIB_THREAD_HPP

#include <halcheck/lib/dag.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <future>
#include <unordered_map>
#include <vector>

namespace halcheck { namespace lib {

/// @brief A serializability checker. Allows users to queue concurrent
///        operations and determine whether the results correspond to the
///        sequential operation of some model.
///
/// @tparam ThreadID User-specified type of thread IDs.
/// @tparam State The type of model used to validate the results of the=
///         concurrent operations.
template<
    typename ThreadID,
    typename State,
    HALCHECK_REQUIRE(lib::is_hashable<ThreadID>()),
    HALCHECK_REQUIRE(lib::is_copyable<State>())>
struct serializability_monitor {
public:
  /// @brief Constructs a new serializability monitor.
  /// @param seed The initial state of the model.
  explicit serializability_monitor(State seed = State()) : _seed(std::move(seed)) {}

  /// @brief Queues a function to run concurrently.
  /// @tparam R A range type of threads.
  /// @tparam F The type of function to execute.
  /// @param range The set of threads to run the function on. All future
  ///              operations that run on any of these threads will be blocked
  ///              until this operation completes.
  /// @param func The system operation to execute. Must return a "validator",
  ///             which is an unary function that performs a corresponding
  ///             "model operation" (the model is given as a reference to this
  ///             function). The validator should check that the result of the
  ///             system operation matches the model operation, and throw an
  ///             exception otherwise.
  ///
  ///             Invocations should look roughly as such:
  ///
  ///             my_monitor.invoke(threads, [&] {
  ///                auto result = my_system.op();
  ///                return [=](State &model) {
  ///                  if (result != model.op())
  ///                     throw some_exception();
  ///                };
  ///             });
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

  /// @brief Waits for all queued operations to complete and verifies their
  ///        results correspond to some sequence of commands on the model.
  ///
  /// @return true The observed results match the model.
  /// @return false The observed results do not match the model.
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
