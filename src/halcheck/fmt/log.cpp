#include <halcheck/fmt/log.hpp>

using namespace halcheck;

const lib::effect<void, const fmt::message &> fmt::log([](const fmt::message &) {});
