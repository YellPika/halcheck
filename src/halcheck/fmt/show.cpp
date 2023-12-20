#include "halcheck/fmt/show.hpp"

#include <halcheck/fmt/indent.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/shrinking.hpp>

#include <doctest/doctest.h>

#include <iomanip>
#include <locale>
#include <ostream>

using namespace halcheck;

void fmt::detail::escape(std::ostream &os, char value) {
  if (value == '\0')
    os << "\\0";
  else if (value == '\n')
    os << "\\n";
  else if (value == '\r')
    os << "\\r";
  else if (value == '\t')
    os << "\\t";
  else if (value == '\'')
    os << "\\'";
  else if (value == '\"')
    os << "\\\"";
  else if (std::isprint(value, os.getloc()))
    os << value;
  else {
    auto flags = os.flags();
    auto reset = lib::finally([&] { os.flags(flags); });
    os << '\\' << std::oct << std::setw(3) << std::setfill('0') << +static_cast<unsigned char>(value);
  }
}

TEST_CASE("show test") {
  REQUIRE_EQ(fmt::to_string(nullptr), "nullptr");
  REQUIRE_EQ(fmt::to_string(static_cast<void *>(nullptr)), "nullptr");
  REQUIRE_EQ(fmt::to_string(std::string("abc\ndef\001", sizeof("abc\ndef\001") - 1)), "\"abc\\ndef\\001\"");

  char array0[] = "Hello";
  REQUIRE_EQ(fmt::to_string(array0), "\"Hello\"");

  int array1[] = {0};
  REQUIRE_EQ(fmt::to_string(array1), "{0}");

  test::check([&] {
    auto x = gen::arbitrary<int>();

    std::ostringstream os;
    os << "new int(" << x << ")";
    REQUIRE_EQ(fmt::to_string(&x), os.str());
  });
}
