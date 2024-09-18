#ifndef HALCHECK_GTEST_PP_HPP
#define HALCHECK_GTEST_PP_HPP

#include <halcheck/gtest/wrap.hpp>
#include <halcheck/lib/pp.hpp>
#include <halcheck/test/strategy.hpp>

#include "gtest/gtest.h"

namespace halcheck { namespace gtest {
test::strategy __attribute__((weak)) default_strategy();
}} // namespace halcheck::gtest

#define HALCHECK_TEST_HELPER(anon, suite, test, strategy)                                                              \
  static void anon();                                                                                                  \
  TEST(suite, test) { ::halcheck::gtest::wrap(strategy)(anon); }                                                       \
  static void anon()

#define HALCHECK_TEST(...)                                                                                             \
  HALCHECK_TEST_HELPER(                                                                                                \
      HALCHECK_CAT(HALCHECK_ANON_FUNC_, __LINE__),                                                                     \
      HALCHECK_1ST(__VA_ARGS__),                                                                                       \
      HALCHECK_2ND(__VA_ARGS__),                                                                                       \
      HALCHECK_3RD(__VA_ARGS__, ::halcheck::gtest::default_strategy()))

#define HALCHECK_TYPED_TEST_HELPER(anon, suite, test, strategy)                                                        \
  template<typename>                                                                                                   \
  static void anon();                                                                                                  \
  TYPED_TEST(suite, test) { ::halcheck::gtest::wrap(strategy)(anon<TypeParam>); /* NOLINT */ }                         \
  template<typename TypeParam>                                                                                         \
  static void anon()

#define HALCHECK_TYPED_TEST(...)                                                                                       \
  HALCHECK_TYPED_TEST_HELPER(                                                                                          \
      HALCHECK_CAT(HALCHECK_ANON_FUNC_, __LINE__),                                                                     \
      HALCHECK_1ST(__VA_ARGS__),                                                                                       \
      HALCHECK_2ND(__VA_ARGS__),                                                                                       \
      HALCHECK_3RD(__VA_ARGS__, ::halcheck::gtest::default_strategy()))

#define HALCHECK_TEST_F_HELPER(anon, fixture, test, strategy)                                                          \
  static void anon();                                                                                                  \
  TEST_F(fixture, test) { ::halcheck::gtest::wrap(strategy)(anon); }                                                   \
  static void anon()

#define HALCHECK_TEST_F(...)                                                                                           \
  HALCHECK_TEST_F_HELPER(                                                                                              \
      HALCHECK_CAT(HALCHECK_ANON_FUNC_, __LINE__),                                                                     \
      HALCHECK_1ST(__VA_ARGS__),                                                                                       \
      HALCHECK_2ND(__VA_ARGS__),                                                                                       \
      HALCHECK_3RD(__VA_ARGS__, ::halcheck::gtest::default_strategy()))

#endif
