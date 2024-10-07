#ifndef HALCHECK_GEN_SHRINKS_HPP
#define HALCHECK_GEN_SHRINKS_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/trie.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace halcheck { namespace gen {

namespace detail {

struct shrink_call {
  std::vector<lib::atom> path;
  std::uintmax_t size;
};

struct shrink_calls {
  std::mutex mutex;
  std::vector<shrink_call> data;
};

struct to_tries {
  to_tries() = default;

  to_tries(
      const std::vector<detail::shrink_call> *calls, const lib::trie<lib::atom, lib::optional<std::uintmax_t>> *input)
      : calls(calls), input(input) {}

  lib::optional<lib::trie<lib::atom, lib::optional<std::uintmax_t>>> operator()() {
    while (call_index < calls->size() && sample_index == (*calls)[call_index].size) {
      sample_index = 0;
      ++call_index;
    }

    if (call_index == calls->size())
      return lib::nullopt;
    else
      return input->set((*calls)[call_index].path, sample_index++);
  }

  const std::vector<detail::shrink_call> *calls = nullptr;
  const lib::trie<lib::atom, lib::optional<std::uintmax_t>> *input = nullptr;
  std::size_t call_index = 0;
  std::size_t sample_index = 0;
};

struct shrink_handler : eff::handler<shrink_handler, gen::shrink_effect, gen::label_effect> {
  shrink_handler(
      lib::trie<lib::atom, lib::optional<std::uintmax_t>> input,
      std::vector<lib::atom> path,
      std::weak_ptr<shrink_calls> calls)
      : input(std::move(input)), path(std::move(path)), calls(std::move(calls)) {}

  lib::optional<std::uintmax_t> operator()(gen::shrink_effect args) override {
    auto output = input.get();
    if (!output && args.size > 0) {
      if (auto locked = calls.lock()) {
        std::lock_guard<std::mutex> _(locked->mutex);
        locked->data.push_back({path, args.size});
      }
    }
    if (args.size == 0 || !output)
      return lib::nullopt;
    else
      return std::min(*output, args.size - 1);
  }

  lib::destructable operator()(gen::label_effect args) override {
    auto prev = input;
    input = input.child(args.value);
    path.push_back(args.value);
    return std::make_pair(gen::label(args.value), lib::finally([&, prev] {
                            path.pop_back();
                            input = prev;
                          }));
  }

  lib::trie<lib::atom, lib::optional<std::uintmax_t>> input;
  std::vector<lib::atom> path;
  std::weak_ptr<shrink_calls> calls;
};

} // namespace detail

template<typename T>
struct shrinks {
public:
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable_r<T, F, Args...>())>
  shrinks(lib::trie<lib::atom, lib::optional<std::uintmax_t>> input, F func, Args &&...args)
      : shrinks(
            std::make_shared<detail::shrink_calls>(), std::move(input), std::move(func), std::forward<Args>(args)...) {}

  lib::add_lvalue_reference_t<const T> get() const { return _value.get(); }
  lib::add_lvalue_reference_t<T> get() { return _value.get(); }

  using children_view = lib::subrange<lib::generate_iterator<detail::to_tries>>;

  children_view children() const {
    return lib::make_subrange(
        lib::make_generate_iterator(detail::to_tries{&_calls, &_input}),
        lib::generate_iterator<detail::to_tries>());
  }

private:
  template<typename F, typename... Args>
  shrinks(
      std::shared_ptr<detail::shrink_calls> calls,
      lib::trie<lib::atom, lib::optional<std::uintmax_t>> input,
      F func,
      Args &&...args)
      : _value(eff::handle(
            [&] { return lib::make_result_holder(func, std::forward<Args>(args)...); },
            detail::shrink_handler(input, {}, calls))),
        _calls([&] {
          std::lock_guard<std::mutex> _(calls->mutex);
          return std::move(calls->data);
        }()),
        _input(std::move(input)) {}

  lib::result_holder<T> _value;
  std::vector<detail::shrink_call> _calls;
  lib::trie<lib::atom, lib::optional<std::uintmax_t>> _input;
};

static const struct {
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::shrinks<lib::invoke_result_t<F, Args...>>
  operator()(lib::trie<lib::atom, lib::optional<std::uintmax_t>> input, F func, Args &&...args) const {
    return gen::shrinks<lib::invoke_result_t<F, Args...>>(
        std::move(input),
        std::move(func),
        std::forward<Args>(args)...);
  }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::shrinks<lib::invoke_result_t<F, Args...>> operator()(F func, Args &&...args) const {
    return gen::shrinks<lib::invoke_result_t<F, Args...>>(
        lib::trie<lib::atom, lib::optional<std::uintmax_t>>(),
        std::move(func),
        std::forward<Args>(args)...);
  }
} make_shrinks;

}} // namespace halcheck::gen

#endif
