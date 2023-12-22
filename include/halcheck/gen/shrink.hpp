#ifndef HALCHECK_GEN_SHRINK_HPP
#define HALCHECK_GEN_SHRINK_HPP

#include <halcheck/gen/group.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/ranges.hpp>

#include <climits>

namespace halcheck { namespace gen {

extern const struct shrink_t : lib::effect<lib::optional<std::uintmax_t>, std::uintmax_t> {
  using effect::effect;

  /// @brief Indicates whether shrinking should occur at the point of invocation.
  /// @param size The number of possible shrinks available at this location.
  /// @return A number in the range [0, size) if shrinking should occur, and
  ///         lib::nullopt otherwise.
  lib::optional<std::uintmax_t> operator()(std::uintmax_t size = 1) const { return effect::operator()(size); }

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
  T with(T root, F func) const {
    auto _ = gen::group();
    while (true) {
      auto children = lib::invoke(func, root);
      auto begin = lib::begin(children);
      auto end = lib::end(children);
      if (auto i = (*this)(std::distance(begin, end)))
        root = std::move(*std::next(begin, *i));
      else
        return root;
    }
  }

  /// @brief Shrinks an integer via binary search towards a given value.
  /// @tparam T The type of integer to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
  T to(T dst, T src) const {
    auto func = [](std::pair<T, T> pair) {
      std::vector<std::pair<T, T>> output;
      while (pair.first != pair.second) {
        auto mid = lib::midpoint(pair.first, pair.second);
        output.push_back({pair.first, mid});
        pair.first = pair.first < pair.second ? mid + 1 : mid - 1;
      }
      return output;
    };
    return this->with(std::make_pair(dst, src), func).second;
  }

  /// @brief Shrinks a floating-point value via binary search.
  /// @tparam T The type of floating-point value to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
  T to(T dst, T src) const {
    auto func = [](std::pair<T, T> pair) {
      std::vector<std::pair<T, T>> output;
      for (std::size_t i = 0; pair.first != pair.second && i < sizeof(T) * CHAR_BIT; i++) {
        auto mid = lib::midpoint(pair.first, pair.second);
        output.push_back({pair.first, mid});
        pair.first = std::nextafter(mid, pair.second);
      }
      return output;
    };
    return this->with(std::make_pair(dst, src), func).second;
  }

  /// @brief Shrinks a number via binary search.
  /// @tparam T The type of number to shrink.
  /// @param dst The value to shrink to.
  /// @param src The value to shrink from.
  /// @return A value in the range [dst, src].
  template<int = 0, typename T, typename U, HALCHECK_REQUIRE(!std::is_same<T, U>())>
  lib::common_type_t<T, U> to(T dst, U src) const {
    return this->to<T>(dst, src);
  }
} shrink;

/// @brief Disables shrinking until the return value is destroyed.
/// @return A value that re-enables shrinking upon destruction.
inline shrink_t::handler prune() {
  return gen::shrink.handle([](std::uintmax_t) { return lib::nullopt; });
}

}} // namespace halcheck::gen

#endif
