#include "halcheck/ext/gtest.hpp"

#include <rang.hpp>

void halcheck::ext::gtest::log(const fmt::message &msg) {
  auto color = ::testing::GTEST_FLAG(color);
  if (color == "auto")
    rang::setControlMode(rang::control::Auto);
  else if (color == "yes")
    rang::setControlMode(rang::control::Force);
  else if (color == "no")
    rang::setControlMode(rang::control::Off);

  std::clog << rang::fg::yellow << "[ HALCHECK ] " << rang::fg::reset;
  std::clog << fmt::show(msg) << "\n";
}
