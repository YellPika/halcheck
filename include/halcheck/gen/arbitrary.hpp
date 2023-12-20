#ifndef HALCHECK_GEN_ARBITRARY_HPP
#define HALCHECK_GEN_ARBITRARY_HPP

#include <halcheck/gen/container.hpp>
#include <halcheck/gen/group.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/optional.hpp>
#include <halcheck/gen/variant.hpp>
#include <halcheck/lib/bit.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <climits>
#include <cstddef>
#include <limits>
#include <utility>

namespace halcheck { namespace gen {

/// @brief A dummy type used to disambiguate calls to arbitrary instances.
/// @tparam T The type whose instance we're interested in.
template<typename T>
struct tag {};

namespace detail {
template<typename T>
struct is_arbitrary_helper;
} // namespace detail

/// @brief Determines if a type supports generating arbitrary values.
/// @tparam T The type of value to query.
template<typename T>
struct is_arbitrary : lib::is_detected<detail::is_arbitrary_helper, T> {};

template<typename T, HALCHECK_REQUIRE(is_arbitrary<T>())>
T arbitrary();

namespace detail {

inline lib::monostate arbitrary(gen::tag<lib::monostate>) { return {}; }

inline bool arbitrary(gen::tag<bool>) { return gen::next(); }

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
T arbitrary(gen::tag<T>) {
  auto _ = gen::group();

  alignas(T) char data[sizeof(T)];
  for (std::size_t i = 0; i < sizeof(T); i++) {
    auto ch = '\0';
    for (int i = 0; i < CHAR_BIT; i++)
      ch = (ch << 1) | gen::next();
    data[i] = ch;
  }

  return gen::shrink(lib::bit_cast<T>(data), T(0));
}

template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>) {
  return T(gen::arbitrary<long long>()) / T(std::numeric_limits<long long>::max()) * std::numeric_limits<T>::max();
}

template<typename T, typename U, HALCHECK_REQUIRE(gen::is_arbitrary<T>()), HALCHECK_REQUIRE(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>) {
  return std::make_pair(gen::arbitrary<T>(), gen::arbitrary<U>());
}

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>) {
  return std::make_tuple(gen::arbitrary<Ts>()...);
}

template<typename T, HALCHECK_REQUIRE(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>) {
  return gen::optional(gen::arbitrary<T>);
}

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>) {
  return gen::variant(gen::arbitrary<Ts>...);
}

template<
    typename Container,
    HALCHECK_REQUIRE(lib::is_insertable<Container>()),
    HALCHECK_REQUIRE(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>) {
  return gen::container<Container>(gen::arbitrary<lib::range_value_t<Container>>);
}

template<typename T>
struct is_arbitrary_helper
    : lib::enable_if_t<std::is_convertible<decltype(arbitrary(gen::tag<T>())), T>() && !std::is_reference<T>()> {};

} // namespace detail

/// @brief Returns an arbitrary value of type T. Requires the existence of a
/// function with the signature arbitrary(halcheck::gen::tag<T>).
/// @tparam T The type of value to return.
template<typename T, lib::enable_if_t<(is_arbitrary<T>()), int>>
T arbitrary() {
  using detail::arbitrary;
  return arbitrary(tag<lib::remove_cv_t<T>>());
}

}} // namespace halcheck::gen

#endif
