#include <halcheck/ext/doctest.hpp>
#include <halcheck/fmt/flatten.hpp>
#include <halcheck/fmt/indent.hpp>
#include <halcheck/gen/container.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/test/check.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace halcheck;

HALCHECK_TEST_CASE("flatten check") {
  auto width = gen::range(0, 20);
  std::ostringstream os;
  fmt::flatten flatten(os, width);

  gen::repeat([&] {
    auto indent = std::string(gen::range(0, width * 2), ' ');
    auto gen_char = [] { return gen::range('a', char('z' + 1)); };
    auto contents = gen::container<std::string>(gen::range(0, width * 2), gen_char);
    os << indent << contents << '\n';
  });
}

TEST_CASE("flatten example") {
  auto print = [](std::size_t width) {
    fmt::flatten flatten(std::cout, width);
    std::cout << "\nWidth: " << width << "\n";
    {
      fmt::indent indent(std::cout, 4);
      std::cout << "int example(int x, int y) { \n";
      std::cout << "int z = x * y; \n";
      std::cout << "int w = x / y; \n";
      std::cout << "return (";
      {
        fmt::indent indent(std::cout, 8);
        std::cout << "foo(z) + \n";
        std::cout << "bar(w)) / 2; }";
      }
    }
    std::cout << "\n\n";
  };

  print(30);
  print(60);
  print(90);
}
