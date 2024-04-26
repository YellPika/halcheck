#include "halcheck/test/record.hpp"

using namespace halcheck;

lib::optional<std::string> test::default_record_directory() {
  static const char *var1 = std::getenv("HALCHECK_RECORD");
  static const char *var2 = std::getenv("HALCHECK_REPLAY");
  if (var1)
    return std::string(var1);
  else if (var2)
    return lib::nullopt;
  else
    return std::string(".");
}
