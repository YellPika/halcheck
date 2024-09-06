#ifndef HALCHECK_GEN_SHRINK_HPP
#define HALCHECK_GEN_SHRINK_HPP

#include <halcheck/eff/api.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <vector>

namespace halcheck { namespace gen {

struct shrink_effect {
  std::uintmax_t size;
  lib::optional<std::uintmax_t> fallback() const { return lib::nullopt; }
};

static struct shrink_t : gen::labelable<shrink_t> {
  using gen::labelable<shrink_t>::operator();

  lib::optional<std::uintmax_t> operator()(std::uintmax_t size = 1) const { return eff::invoke<shrink_effect>(size); }

  template<
      typename T,
      typename I,
      HALCHECK_REQUIRE(lib::is_forward_iterator<I>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::iter_reference_t<I>(), T &>())>
  T &operator()(T &root, I begin, I end) const {
    if (auto index = eff::invoke<shrink_effect>(std::distance(begin, end)))
      return *std::next(begin, index);
    else
      return root;
  }

  template<
      typename T,
      typename R,
      HALCHECK_REQUIRE(lib::is_range<R>()),
      HALCHECK_REQUIRE(std::is_convertible<lib::range_reference_t<R>(), T &>())>
  T &operator()(T &root, const R &range) const {
    return (*this)(root, lib::begin(range), lib::end(range));
  }

  template<typename T>
  T &operator()(T &root, const std::initializer_list<T> &range) const {
    return (*this)(root, lib::begin(range), lib::end(range));
  }

  /// @brief QuickCheck-style shrinking.
  /// @tparam T The type of value to shrink.
  /// @tparam F the type of function describing the possible ways to shrink.
  /// @param root The initial value to return.
  /// @param func A function describing the possible ways to shrink a value.
  /// @return root or one of its shrinks.
  template<
      typename T,
      typename F,
      HALCHECK_REQUIRE(lib::is_invocable<F, T>()),
      HALCHECK_REQUIRE(lib::is_range<lib::invoke_result_t<F, T>>()),
      HALCHECK_REQUIRE(std::is_assignable<T &, lib::range_value_t<lib::invoke_result_t<F, T>>>())>
  T operator()(T root, F func) const {
    for (std::uintmax_t i = 0;; i++) {
      auto _ = gen::label(lib::number(i));

      auto children = lib::invoke(func, root);
      auto begin = lib::begin(children);
      auto end = lib::end(children);
      if (auto j = (*this)(std::distance(begin, end)))
        root = std::move(*std::next(begin, *j));
      else
        break;
    }

    return root;
  }
} shrink;

static struct {
  /// @brief Shrinks an integer via binary search towards a given value.
  /// @tparam T The type of integer to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
  T operator()(T dst, T src) const {
    auto func = [&](std::pair<T, T> pair) {
      std::vector<std::pair<T, T>> output;
      while (pair.first != pair.second) {
        auto mid = lib::midpoint(pair.first, pair.second);
        output.push_back({pair.first, mid});
        pair.first = pair.first < pair.second ? mid + 1 : mid - 1;
      }
      return output;
    };
    return gen::shrink(std::make_pair(dst, src), func).second;
  }

  /// @brief Shrinks a floating-point value via binary search.
  /// @tparam T The type of floating-point value to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
  T operator()(T dst, T src) const {
    auto func = [](std::pair<T, T> pair) {
      std::vector<std::pair<T, T>> output;
      for (std::size_t i = 0; pair.first != pair.second && i < sizeof(T) * CHAR_BIT; i++) {
        auto mid = lib::midpoint(pair.first, pair.second);
        output.push_back({pair.first, mid});
        pair.first = std::nextafter(mid, pair.second);
      }
      return output;
    };
    return gen::shrink(std::make_pair(dst, src), func).second;
  }

  /// @brief Shrinks a number via binary search.
  /// @tparam T The type of number to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<typename T, typename U, HALCHECK_REQUIRE(!std::is_same<T, U>())>
  lib::common_type_t<T, U> operator()(T dst, U src) const {
    return this->operator()<lib::common_type_t<T, U>>(dst, src);
  }
} shrink_to;

static struct noshrink_t : gen::labelable<noshrink_t> {
  using gen::labelable<noshrink_t>::operator();

  struct handler : eff::handler<handler, gen::shrink_effect> {
    lib::optional<std::uintmax_t> operator()(gen::shrink_effect) override { return lib::nullopt; }
  };

  template<typename F, typename... Args, HALCHECK_REQUIRE(lib::is_invocable<F, Args...>())>
  lib::invoke_result_t<F, Args...> operator()(F func, Args &&...args) const {
    return eff::handle([&] { return lib::invoke(std::move(func), std::forward<Args>(args)...); }, handler());
  }
} noshrink;

}} // namespace halcheck::gen

#endif
