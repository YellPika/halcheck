#ifndef HALCHECK_GEN_ARBITRARY_HPP
#define HALCHECK_GEN_ARBITRARY_HPP

#include <halcheck/gen/container.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/optional.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/shrink.hpp>
#include <halcheck/gen/variant.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/bit.hpp>
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

/// @brief A dummy type used to disambiguate calls to arbitrary instances.
/// @tparam T The type whose instance we're interested in.
template<typename T>
struct tag {};

template<typename T>
struct is_arbitrary;

namespace detail {
inline lib::monostate arbitrary(gen::tag<lib::monostate>, lib::atom);

inline bool arbitrary(gen::tag<bool>, lib::atom);

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
T arbitrary(gen::tag<T>, lib::atom);

template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>, lib::atom);

template<typename T, typename U, HALCHECK_REQUIRE(gen::is_arbitrary<T>()), HALCHECK_REQUIRE(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>, lib::atom);

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>, lib::atom);

template<typename T, HALCHECK_REQUIRE(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>, lib::atom);

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>, lib::atom);

template<
    typename Container,
    HALCHECK_REQUIRE(lib::is_insertable_range<Container>()),
    HALCHECK_REQUIRE(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>, lib::atom);

template<typename T>
using is_arbitrary_helper = lib::enable_if_t<
    std::is_convertible<decltype(arbitrary(gen::tag<T>(), lib::atom())), T>() && !std::is_reference<T>()>;
} // namespace detail

/// @brief Determines if a type supports generating arbitrary values.
/// @tparam T The type of value to query.
template<typename T>
struct is_arbitrary : lib::is_detected<detail::is_arbitrary_helper, lib::remove_cv_t<T>> {};

template<typename T, HALCHECK_REQUIRE(is_arbitrary<T>())>
T arbitrary(lib::atom id) {
  using detail::arbitrary;
  return arbitrary(gen::tag<lib::decay_t<T>>(), id);
}

namespace detail {

inline lib::monostate arbitrary(gen::tag<lib::monostate>, lib::atom) { return {}; }

inline bool arbitrary(gen::tag<bool>, lib::atom id) { return gen::sample(id, 1) > 0 && !gen::shrink(id); }

template<typename T, HALCHECK_REQUIRE_(std::is_integral<T>())>
T arbitrary(gen::tag<T>, lib::atom id) {
  auto src = gen::sample(id);
  T dest;
  std::memcpy(&dest, &src, sizeof(T));
  return gen::shrink_to(id, T(0), dest);
}

template<typename T, HALCHECK_REQUIRE_(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>, lib::atom id) {
  return T(gen::arbitrary<std::intmax_t>(id)) / T(std::numeric_limits<std::intmax_t>::max()) *
         std::numeric_limits<T>::max();
}

template<typename T, typename U, HALCHECK_REQUIRE_(gen::is_arbitrary<T>()), HALCHECK_REQUIRE_(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>, lib::atom id) {
  auto _ = gen::label(id);
  return std::make_pair(gen::arbitrary<T>(lib::number(0)), gen::arbitrary<U>(lib::number(1)));
}

template<typename... Ts, HALCHECK_REQUIRE_(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>, lib::atom id) {
  auto _ = gen::label(id);
  std::uintmax_t i = 0;
  return std::tuple<Ts...>{gen::arbitrary<Ts>(lib::number(i++))...};
}

template<typename T, HALCHECK_REQUIRE_(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>, lib::atom id) {
  return gen::optional(id, gen::arbitrary<T>);
}

template<typename... Ts, HALCHECK_REQUIRE_(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>, lib::atom id) {
  return gen::variant(id, gen::arbitrary<Ts>...);
}

template<
    typename Container,
    HALCHECK_REQUIRE_(lib::is_insertable_range<Container>()),
    HALCHECK_REQUIRE_(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>, lib::atom id) {
  return gen::container<Container>(id, gen::arbitrary<lib::range_value_t<Container>>);
}

} // namespace detail

}} // namespace halcheck::gen

#endif
