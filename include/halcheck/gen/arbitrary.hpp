#ifndef HALCHECK_GEN_ARBITRARY_HPP
#define HALCHECK_GEN_ARBITRARY_HPP

/**
 * @defgroup gen-arbitrary gen/arbitrary
 * @brief Generating arbitrary values of a type.
 * @ingroup gen
 */

#include <halcheck/gen/container.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/optional.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/variant.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/iterator.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <tuple>
#include <utility>

namespace halcheck { namespace gen {

namespace detail {
/**
 * @brief A dummy type used to disambiguate calls to arbitrary instances.
 * @tparam T The type whose instance we're interested in.
 * @ingroup gen-arbitrary
 */
template<typename T>
struct tag {};
} // namespace detail

using detail::tag;

namespace detail {
template<typename T>
using is_arbitrary_helper = lib::enable_if_t<
    std::is_convertible<decltype(arbitrary(gen::tag<T>(), lib::atom())), T>() && !std::is_reference<T>()>;
} // namespace detail

/**
 * @brief Determines if a type supports generating arbitrary values.
 * @tparam T The type of value to query.
 * @details The type @p T satisfies @ref is_arbitrary if and only if it is not a reference type and the expression
 * `arbitrary(gen::tag<T>(), lib::atom())` returns a value convertible to type @p T. Note that, by default, this holds
 * for the following cases:
 * - @p T is `lib::monostate`
 * - @p T is `bool`
 * - `std::is_integral<T>()` holds.
 * - `std::is_floating_point<T>()` holds.
 * - @p T is `std::pair<U, V>`, where `gen::is_arbitrary<U>()` and `gen::is_arbitrary<V>()` hold
 * - @p T is `std::tuple<Args...>`, where `gen::is_arbitrary<U>()` holds for all `U` in `Args...`
 * - @p T is `lib::optional<U>`, where `gen::is_arbitrary<U>()` holds
 * - @p T is `std::variant<Args...>`, where `gen::is_arbitrary<U>()` holds for all `U` in `Args...`
 * - `lib::is_insertable_range<T>()` and `gen::is_arbitrary<lib::range_value_t<Container>>()` hold
 * @ingroup gen-arbitrary
 */
template<typename T>
struct is_arbitrary : lib::is_detected<detail::is_arbitrary_helper, lib::remove_cv_t<T>> {};

/**
 * @brief Generates an arbitrary value of a given type.
 * @tparam T The type of value to obtain.
 * @param id A unique id for the generated value.
 * @return `arbitrary(lib::tag<lib::decay_t<T>>(), id)`
 * @details This overload participates in overload resolution only if `gen::is_arbitrary<T>()` is satisfied.
 * @ingroup gen-arbitrary
 */
template<typename T, HALCHECK_REQUIRE(is_arbitrary<T>())>
T arbitrary(lib::atom id) {
  return arbitrary(gen::tag<lib::decay_t<T>>(), id);
}

namespace detail {
inline lib::monostate arbitrary(gen::tag<lib::monostate>, lib::atom) { return {}; }

inline bool arbitrary(gen::tag<bool>, lib::atom id) { return gen::sample(id, 1) > 0 && !gen::shrink(id); }

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
T arbitrary(gen::tag<T>, lib::atom id) {
  auto src = gen::sample(id);
  T dst;
  std::memcpy(&dst, &src, sizeof(dst));
  return gen::shrink_to(id, T(0), dst);
}

template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>, lib::atom id) {
  return T(gen::arbitrary<std::intmax_t>(id)) / T(std::numeric_limits<std::intmax_t>::max()) *
         std::numeric_limits<T>::max();
}

template<typename T, typename U, HALCHECK_REQUIRE(gen::is_arbitrary<T>()), HALCHECK_REQUIRE(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>, lib::atom id) {
  auto _ = gen::label(id);
  return std::make_pair(gen::arbitrary<T>(0), gen::arbitrary<U>(1));
}

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>, lib::atom id) {
  auto _ = gen::label(id);
  std::uintmax_t i = 0;
  return std::tuple<Ts...>{gen::arbitrary<Ts>(i++)...};
}

template<typename T, HALCHECK_REQUIRE(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>, lib::atom id) {
  return gen::optional(id, gen::arbitrary<T>);
}

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>, lib::atom id) {
  return gen::variant(id, gen::arbitrary<Ts>...);
}

template<
    typename Container,
    HALCHECK_REQUIRE(lib::is_insertable_range<Container>()),
    HALCHECK_REQUIRE(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>, lib::atom id) {
  return gen::container<Container>(id, gen::arbitrary<lib::range_value_t<Container>>);
}
} // namespace detail

}} // namespace halcheck::gen

#endif
