#ifndef HALCHECK_TEST_SAMPLER_HPP
#define HALCHECK_TEST_SAMPLER_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <functional>
#include <type_traits>

namespace halcheck { namespace test {

template<typename T>
struct is_sampler : lib::is_invocable<T, std::function<void()>> {};

}} // namespace halcheck::test

#endif
