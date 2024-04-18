#include "halcheck/ext/gtest.hpp"

#include <rang.hpp>

void halcheck::ext::gtest::log(const fmt::message &msg) {
  auto color = ::testing::GTEST_FLAG(color);
  std::transform(color.begin(), color.end(), color.begin(), [](char c) { return std::tolower(c); });
  if (color == "auto")
    rang::setControlMode(rang::control::Auto);
  else if (color == "yes" || color == "true" || color == "t" || color == "1")
    rang::setControlMode(rang::control::Force);
  else
    rang::setControlMode(rang::control::Off);

  auto indent = [](std::ostream &os) { os << rang::fg::yellow << "[ HALCHECK ] " << rang::fg::reset; };
  indent(std::clog);
  {
    fmt::indent _(std::clog, indent);
    std::clog << fmt::show(msg);
  }
  std::clog << "\n";
}
