#include "halcheck/fmt/type.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/fmt/flatten.hpp>
#include <halcheck/fmt/indent.hpp>
#include <halcheck/fmt/infer.hpp>
#include <halcheck/fmt/show.hpp>

#include <iostream>

using namespace halcheck;

namespace {
enum example0 { X, Y, Z };

std::ostream &operator<<(std::ostream &os, fmt::type<example0>) { return os << "example0"; }

std::ostream &operator<<(std::ostream &os, fmt::tag<example0> value) {
  switch (value.value) {
  case X:
    return os << 'X';
  case Y:
    return os << 'Y';
  case Z:
    return os << 'Z';
  }

  return os << "example0(" << value.value << ")";
}

enum example1 { A, B, C };

std::ostream &operator<<(std::ostream &os, fmt::type<example1>) { return os << "example1"; }

template<typename = int>
struct example2 {};

std::ostream &operator<<(std::ostream &os, fmt::type_fun<example2>) { return os << "example2"; }
} // namespace

TEST_CASE("type example") {
  auto value = {
      std::make_tuple(std::string("a"), 0, 0.15f, X, A),
      std::make_tuple(std::string("ab"), -1, 0.2f, Y, B),
      std::make_tuple(std::string("abc"), -2, 3.0f, Z, C),
  };

  fmt::flatten flatten(std::cout);
  fmt::infer(std::cout, true);
  std::cout << fmt::show(value) << '\n';
  fmt::infer(std::cout, false);
  std::cout << fmt::show(value) << '\n';

  std::cout << fmt::type<example2<>>() << '\n';
}
