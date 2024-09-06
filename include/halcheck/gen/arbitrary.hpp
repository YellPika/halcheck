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
inline lib::monostate arbitrary(gen::tag<lib::monostate>);

inline bool arbitrary(gen::tag<bool>);

template<typename T, HALCHECK_REQUIRE(std::is_integral<T>())>
T arbitrary(gen::tag<T>);

template<typename T, HALCHECK_REQUIRE(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>);

template<typename T, typename U, HALCHECK_REQUIRE(gen::is_arbitrary<T>()), HALCHECK_REQUIRE(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>);

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>);

template<typename T, HALCHECK_REQUIRE(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>);

template<typename... Ts, HALCHECK_REQUIRE(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>);

template<
    typename Container,
    HALCHECK_REQUIRE(lib::is_insertable<Container>()),
    HALCHECK_REQUIRE(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>);

template<typename T>
using is_arbitrary_helper =
    lib::enable_if_t<std::is_convertible<decltype(arbitrary(gen::tag<T>())), T>() && !std::is_reference<T>()>;
} // namespace detail

/// @brief Determines if a type supports generating arbitrary values.
/// @tparam T The type of value to query.
template<typename T>
struct is_arbitrary : lib::is_detected<detail::is_arbitrary_helper, lib::remove_cv_t<T>> {};

/// @brief Returns an arbitrary value of type T. Requires the existence of a
/// function with the signature arbitrary(halcheck::gen::tag<T>).
/// @tparam T The type of value to return.
template<typename T, HALCHECK_REQUIRE(is_arbitrary<T>())>
T arbitrary() {
  using detail::arbitrary;
  return arbitrary(tag<lib::remove_cv_t<T>>());
}

namespace detail {

inline lib::monostate arbitrary(gen::tag<lib::monostate>) { return {}; }

inline bool arbitrary(gen::tag<bool>) { return gen::sample(1) > 0 && !gen::shrink(); }

template<typename T, HALCHECK_REQUIRE_(std::is_integral<T>())>
T arbitrary(gen::tag<T>) {
  auto src = gen::sample();
  T dest;
  std::memcpy(&dest, &src, sizeof(T));
  return gen::shrink_to(T(0), dest);
}

template<typename T, HALCHECK_REQUIRE_(std::is_floating_point<T>())>
T arbitrary(gen::tag<T>) {
  return T(gen::arbitrary<std::intmax_t>()) / T(std::numeric_limits<std::intmax_t>::max()) *
         std::numeric_limits<T>::max();
}

template<typename T, typename U, HALCHECK_REQUIRE_(gen::is_arbitrary<T>()), HALCHECK_REQUIRE_(gen::is_arbitrary<U>())>
std::pair<T, U> arbitrary(gen::tag<std::pair<T, U>>) {
  using namespace lib::literals;
  return std::make_pair(gen::label(0_n, gen::arbitrary<T>), gen::label(1_n, gen::arbitrary<U>));
}

template<typename... Ts, HALCHECK_REQUIRE_(lib::conjunction<gen::is_arbitrary<Ts>...>())>
std::tuple<Ts...> arbitrary(gen::tag<std::tuple<Ts...>>) {
  std::uintmax_t i = 0;
  return std::tuple<Ts...>{gen::label(lib::number(i++), gen::arbitrary<Ts>)...};
}

template<typename T, HALCHECK_REQUIRE_(gen::is_arbitrary<T>())>
lib::optional<T> arbitrary(gen::tag<lib::optional<T>>) {
  return gen::optional(gen::arbitrary<T>);
}

template<typename... Ts, HALCHECK_REQUIRE_(lib::conjunction<gen::is_arbitrary<Ts>...>())>
lib::variant<Ts...> arbitrary(gen::tag<lib::variant<Ts...>>) {
  return gen::variant(gen::arbitrary<Ts>...);
}

template<
    typename Container,
    HALCHECK_REQUIRE_(lib::is_insertable<Container>()),
    HALCHECK_REQUIRE_(gen::is_arbitrary<lib::range_value_t<Container>>())>
Container arbitrary(gen::tag<Container>) {
  return gen::container<Container>(gen::arbitrary<lib::range_value_t<Container>>);
}

} // namespace detail

extern template bool arbitrary<bool>();
extern template unsigned char arbitrary<unsigned char>();
extern template signed char arbitrary<signed char>();
extern template char arbitrary<char>();
extern template unsigned short arbitrary<unsigned short>();
extern template signed short arbitrary<signed short>();
extern template unsigned int arbitrary<unsigned int>();
extern template signed int arbitrary<signed int>();
extern template unsigned long arbitrary<unsigned long>();
extern template signed long arbitrary<signed long>();
extern template unsigned long long arbitrary<unsigned long long>();
extern template signed long long arbitrary<signed long long>();
extern template float arbitrary<float>();
extern template double arbitrary<double>();
extern template long double arbitrary<long double>();

}} // namespace halcheck::gen

#endif
