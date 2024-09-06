#ifndef HALCHECK_SHRINK_SHRINKS_HPP
#define HALCHECK_SHRINK_SHRINKS_HPP

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

struct shrink_index_to_trie {
  lib::trie<lib::atom, lib::optional<std::uintmax_t>> operator()(std::uintmax_t index) const {
    return input.set(path, index);
  }

  const std::vector<lib::atom> &path;
  const lib::trie<lib::atom, lib::optional<std::uintmax_t>> &input;
};

struct shrink_call_to_tries {
  lib::transform_view<lib::integral_view<std::uintmax_t>, detail::shrink_index_to_trie>
  operator()(const detail::shrink_call &call) const {
    return lib::make_transform_view(
        lib::make_integral_view(std::uintmax_t(0), call.size),
        detail::shrink_index_to_trie{call.path, input});
  }

  const lib::trie<lib::atom, lib::optional<std::uintmax_t>> &input;
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
struct shrinks_t {
public:
  explicit shrinks_t(
      lib::trie<lib::atom, lib::optional<std::uintmax_t>> input,
      lib::result_holder<T> value,
      std::vector<detail::shrink_call> calls)
      : _input(std::move(input)), _value(std::move(value)), _calls(std::move(calls)) {}

  typename std::add_lvalue_reference<const T>::type get() const { return _value.get(); }
  typename std::add_lvalue_reference<T>::type get() { return _value.get(); }

  std::size_t size() const {
    std::size_t output = 0;
    for (auto &&call : _calls)
      output += call.size;
    return output;
  }

  using children_view =
      lib::concat_view<lib::transform_view<const std::vector<detail::shrink_call>, detail::shrink_call_to_tries>>;

  children_view children() const {
    return lib::make_concat_view(lib::make_transform_view(_calls, detail::shrink_call_to_tries{_input}));
  }

private:
  lib::trie<lib::atom, lib::optional<std::uintmax_t>> _input;
  lib::result_holder<T> _value;
  std::vector<detail::shrink_call> _calls;
};

static const struct {
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::shrinks_t<lib::invoke_result_t<F>>
  operator()(lib::trie<lib::atom, lib::optional<std::uintmax_t>> input, F func, Args &&...args) const {
    auto calls = std::make_shared<detail::shrink_calls>();
    auto value = eff::handle(
        [&] { return lib::make_result_holder(func, std::forward<Args>(args)...); },
        detail::shrink_handler(input, {}, calls));

    std::lock_guard<std::mutex> _(calls->mutex);
    return gen::shrinks_t<lib::invoke_result_t<F>>(std::move(input), std::move(value), std::move(calls->data));
  }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::shrinks_t<lib::invoke_result_t<F>> operator()(F func, Args &&...args) const {
    return (*this)(lib::trie<lib::atom, lib::optional<std::uintmax_t>>(), std::move(func), std::forward<Args>(args)...);
  }
} shrinks;

}} // namespace halcheck::gen

#endif
