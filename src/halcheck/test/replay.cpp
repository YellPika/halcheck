#include "halcheck/test/replay.hpp"

using namespace halcheck;

lib::optional<std::string> test::default_replay_directory() {
  static const char *var = std::getenv("HALCHECK_REPLAY");
  if (var)
    return std::string(var);
  else
    return lib::nullopt;
}
