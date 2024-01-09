#include <halcheck/ext/doctest.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/test/check.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

using namespace halcheck;

namespace example {

struct type : std::tuple<bool> {
  using tuple::tuple;
};

type arbitrary(gen::tag<type>) { return type{true}; }

} // namespace example

example::type arbitrary(gen::tag<example::type>) { return example::type{false}; }

HALCHECK_TEST_CASE_TEMPLATE_DEFINE("gen::arbitrary", T, gen_arbitrary) { gen::guard(gen::arbitrary<T>() != T()); }

TEST_SUITE("gen::arbitrary") {
  TEST_CASE("gen::arbitrary calls arbitrary(tag<T>)") { CHECK(std::get<0>(gen::arbitrary<example::type>())); }

  TEST_CASE("gen::arbitrary works with using statements") {
    using gen::arbitrary;
    CHECK(std::get<0>(arbitrary<example::type>()));
  }

  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, bool);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, char);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, signed char);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, unsigned char);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, signed short);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, unsigned short);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, signed int);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, unsigned int);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, signed long);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, unsigned long);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, signed long long);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, unsigned long long);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, float);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, double);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, long double);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, std::pair<std::pair<int, bool>, example::type>);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, lib::optional<example::type>);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, lib::variant<example::type, int>);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, std::vector<example::type>);
  TEST_CASE_TEMPLATE_INVOKE(gen_arbitrary, std::map<std::string, example::type>);
}
