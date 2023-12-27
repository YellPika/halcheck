#ifndef HALCHECK_TEST_STRATEGY_HPP
#define HALCHECK_TEST_STRATEGY_HPP

#include <halcheck/lib/functional.hpp>

#include <functional>
#include <type_traits>

namespace halcheck { namespace test {

template<typename T>
struct is_strategy : lib::is_invocable<T, std::function<void()>> {};

}} // namespace halcheck::test

#endif
