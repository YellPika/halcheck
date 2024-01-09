#include <halcheck/ext/doctest.hpp>
#include <halcheck/fmt/indent.hpp>
#include <halcheck/fmt/show.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/check.hpp>

#include <iomanip>
#include <locale>

using namespace halcheck;

TEST_CASE("show test") {
  CHECK_EQ(fmt::to_string(nullptr), "nullptr");
  CHECK_EQ(fmt::to_string(static_cast<void *>(nullptr)), "nullptr");
  CHECK_EQ(fmt::to_string(std::string("abc\ndef\001", sizeof("abc\ndef\001") - 1)), "\"abc\\ndef\\001\"");

  char array0[] = "Hello";
  CHECK_EQ(fmt::to_string(array0), "\"Hello\"");

  int array1[] = {0};
  CHECK_EQ(fmt::to_string(array1), "{0}");

  test::check([&] {
    auto x = gen::arbitrary<int>();

    std::ostringstream os;
    os << "new int(" << x << ")";
    CHECK_EQ(fmt::to_string(&x), os.str());
  });
}
