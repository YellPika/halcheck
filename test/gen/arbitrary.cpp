#include <halcheck.hpp>
#include <halcheck/gtest.hpp>

#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace halcheck;

namespace example {

struct type : std::tuple<bool> {
  using tuple::tuple;
};

type arbitrary(gen::tag<type>, lib::atom) { return type{true}; }

} // namespace example

example::type arbitrary(gen::tag<example::type>, lib::atom) { return example::type{false}; }

template<typename TypeParam>
struct Arbitrary : public testing::Test {};

using Types = testing::Types<
    bool,
    char,
    unsigned char,
    signed char,
    unsigned char,
    signed short,
    unsigned short,
    signed int,
    unsigned long,
    signed long,
    unsigned long long,
    signed long long,
    float,
    double,
    long double,
    std::tuple<int>,
    std::tuple<example::type, int>,
    std::pair<std::pair<int, bool>, example::type>,
    lib::optional<int>,
    lib::variant<int>,
    lib::variant<example::type, int>,
    std::vector<example::type>,
    std::map<std::string, example::type>>;
TYPED_TEST_SUITE(Arbitrary, Types, );

HALCHECK_TYPED_TEST(Arbitrary, Arbitrary) {
  using namespace lib::literals;
  auto value1 = gen::arbitrary<TypeParam>("a"_s);
  auto value2 = gen::arbitrary<TypeParam>("b"_s);
  gen::guard(value1 != value2);
}
