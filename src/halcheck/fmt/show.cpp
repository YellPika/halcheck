#include "halcheck/fmt/show.hpp"

#include <halcheck/fmt/indent.hpp>
#include <halcheck/gen/arbitrary.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/test/check.hpp>

#include <iomanip>

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
