#include <halcheck.hpp>
#include <halcheck/glog.hpp>
#include <halcheck/gtest.hpp>

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

using namespace halcheck;

HALCHECK_TEST(Container, Shrinks) {
  using namespace lib::literals;

  auto func = [] {
    auto _ = gen::label("it"_s);
    return gen::container<std::vector<bool>>(gen::arbitrary);
  };

  auto prev = gen::make_shrinks(func);
  for (std::uintmax_t i = 0; !prev.get().empty(); i++) {
    LOG(INFO) << "prev: " << testing::PrintToString(prev.get());
    ASSERT_NE(prev.children().begin(), prev.children().end());
    auto next = gen::make_shrinks(gen::element_of(lib::number(i), prev.children()), func);
    if (next.get().size() == prev.get().size()) {
      std::uintmax_t j = 0;
      while (j < prev.get().size() && next.get()[j] == prev.get()[j])
        ++j;
      ASSERT_LT(j, prev.get().size());
      ASSERT_LT(next.get()[j], prev.get()[j]);
      for (j++; j < prev.get().size(); j++)
        ASSERT_EQ(next.get()[j], prev.get()[j]);
    } else {
      ASSERT_LT(next.get().size(), prev.get().size());
    }
    prev = std::move(next);
  }

  ASSERT_EQ(prev.children().begin(), prev.children().end());
}

HALCHECK_TEST(Container, ForwardShrinks) {
  using namespace lib::literals;

  auto func = [] {
    auto _ = gen::label("it"_s);
    return gen::container<std::vector<bool>>(gen::arbitrary);
  };

  auto prev = gen::make_forward_shrinks(func);
  for (std::uintmax_t i = 0; !prev.children().empty(); i++) {
    LOG(INFO) << "prev: " << testing::PrintToString(prev.get()) << ", shrinks: " << prev.children().size();
    ASSERT_NE(prev.children().begin(), prev.children().end());
    auto next = gen::make_forward_shrinks(gen::element_of(lib::number(i), prev.children()), func);
    if (next.get().size() == prev.get().size()) {
      std::uintmax_t j = 0;
      while (j < prev.get().size() && next.get()[j] == prev.get()[j])
        ++j;
      ASSERT_LT(j, prev.get().size());
      ASSERT_LT(next.get()[j], prev.get()[j]);
      for (j++; j < prev.get().size(); j++)
        ASSERT_EQ(next.get()[j], prev.get()[j]);
    } else {
      ASSERT_LT(next.get().size(), prev.get().size());
    }
    prev = std::move(next);
  }

  ASSERT_EQ(prev.children().begin(), prev.children().end());
}
