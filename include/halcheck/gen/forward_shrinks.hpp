#ifndef HALCHECK_GEN_FORWARD_SHRINKS_HPP
#define HALCHECK_GEN_FORWARD_SHRINKS_HPP

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
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace halcheck { namespace gen {

namespace detail {

struct forward_shrink_append {
  std::vector<std::uintmax_t> operator()(std::uintmax_t value) const {
    auto output = *input;
    output.push_back(value);
    return output;
  }

  const std::vector<std::uintmax_t> *input;
};

struct forward_shrink_handler : eff::handler<forward_shrink_handler, gen::shrink_effect> {
  explicit forward_shrink_handler(std::vector<std::uintmax_t> input)
      : data(new data_t{std::move(input), 0, 0}), origin(std::this_thread::get_id()) {}

  lib::optional<std::uintmax_t> operator()(gen::shrink_effect args) override {
    if (std::this_thread::get_id() != origin)
      throw std::runtime_error("cannot use forward shrinking with multiple threads");

    if (data->index >= data->input.size()) {
      data->remaining += args.size;
      return lib::nullopt;
    }

    if (data->input[data->index] >= args.size) {
      data->input[data->index] -= args.size;
      return lib::nullopt;
    }

    return data->input[data->index++];
  }

  struct data_t {
    std::vector<std::uintmax_t> input;
    std::size_t index;
    std::size_t remaining;
  };

  std::shared_ptr<data_t> data;
  std::thread::id origin;
};

} // namespace detail

template<typename T>
struct forward_shrinks {
public:
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable_r<T, F, Args...>())>
  forward_shrinks(std::vector<std::uintmax_t> input, F func, Args &&...args)
      : forward_shrinks(detail::forward_shrink_handler(input), input, std::move(func), std::forward<Args>(args)...) {}

  typename std::add_lvalue_reference<const T>::type get() const { return _value.get(); }
  typename std::add_lvalue_reference<T>::type get() { return _value.get(); }

  using children_view = lib::transform_view<lib::integral_view<std::uintmax_t>, detail::forward_shrink_append>;

  children_view children() const {
    return lib::make_transform_view(
        lib::make_integral_view(std::uintmax_t(0), _remaining),
        detail::forward_shrink_append{&_input});
  }

private:
  template<typename F, typename... Args>
  forward_shrinks(detail::forward_shrink_handler handler, std::vector<uintmax_t> input, F func, Args &&...args)
      : _value(eff::handle([&] { return lib::make_result_holder(func, std::forward<Args>(args)...); }, handler)),
        _input(std::move(input)), _remaining(handler.data->remaining) {}

  lib::result_holder<T> _value;
  std::vector<std::uintmax_t> _input;
  std::uintmax_t _remaining;
};

static const struct {
  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::forward_shrinks<lib::invoke_result_t<F, Args...>>
  operator()(std::vector<std::uintmax_t> input, F func, Args &&...args) const {
    return gen::forward_shrinks<lib::invoke_result_t<F, Args...>>(
        std::move(input),
        std::move(func),
        std::forward<Args>(args)...);
  }

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  gen::forward_shrinks<lib::invoke_result_t<F, Args...>> operator()(F func, Args &&...args) const {
    return gen::forward_shrinks<lib::invoke_result_t<F, Args...>>(
        std::vector<std::uintmax_t>(),
        std::move(func),
        std::forward<Args>(args)...);
  }
} make_forward_shrinks;

}} // namespace halcheck::gen

#endif
