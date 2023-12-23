#include "halcheck/fmt/flatten.hpp"

#include <halcheck/ext/doctest.hpp>
#include <halcheck/fmt/indent.hpp>
#include <halcheck/fmt/show.hpp>
#include <halcheck/gen/container.hpp>
#include <halcheck/gen/next.hpp>
#include <halcheck/gen/range.hpp>
#include <halcheck/gen/weight.hpp>
#include <halcheck/test/check.hpp>
#include <halcheck/test/shrinking.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace halcheck;

namespace {
class streambuf : public std::streambuf {
public:
  explicit streambuf(std::streambuf *base, std::size_t width) : _base(base), _width(width) {}

  ~streambuf() override {
    if (_indent > 0 || !_content.empty())
      newline();

    while (flatten())
      ;

    flush();
  }

  int overflow(int ch) noexcept override {
    if (ch == traits_type::eof())
      return sync();

    if (ch == ' ' && _content.empty()) {
      _indent++;
      return 0;
    }

    if (ch == '\n')
      newline();
    else
      _content.push_back(ch);

    return 0;
  }

  int sync() override { return _base->pubsync(); }

  streambuf *setbuf(char_type *s, std::streamsize n) override {
    _base->pubsetbuf(s, n);
    return this;
  }

  std::streambuf *base() const { return _base; }

private:
  void newline() {
    while (!_buffer.empty() && _buffer.back().indent > _indent) {
      if (!flatten())
        flush();
    }

    if (_buffer.empty() || _buffer.back().indent < _indent)
      _buffer.push_back({_indent, {std::move(_content)}});
    else {
      if (_buffer.size() == 1) {
        print(_buffer.back());
        _buffer.back().content.clear();
      }
      _buffer.back().content.push_back(_content);
    }

    _indent = 0;
    _content.clear();
  }

  bool flatten() noexcept {
    if (_buffer.size() <= 1)
      return false;

    std::string flattened;
    for (auto &&line : _buffer.back().content)
      flattened += line;

    auto &next = _buffer[_buffer.size() - 2];
    if (next.indent + next.content.back().size() + flattened.size() > _width)
      return false;

    _buffer.pop_back();
    _buffer.back().content.back() += flattened;
    return true;
  }

  void flush() noexcept {
    for (auto &&scope : _buffer)
      print(scope);
    _buffer.clear();
  }

  struct scope {
    std::size_t indent;
    std::vector<std::string> content;
  };

  void print(const scope &scope) const {
    std::string indent(scope.indent, ' ');
    for (auto &&line : scope.content) {
      _base->sputn(indent.data(), indent.size());
      _base->sputn(line.data(), line.size());
      _base->sputc('\n');
    }
  }

  std::streambuf *_base;
  std::size_t _width;

  // INVARIANT: _buffer is increasing order by indentation
  std::vector<scope> _buffer;

  std::size_t _indent = 0;
  std::string _content;
};
} // namespace

fmt::flatten::flatten(std::ostream &os, std::size_t width) : _os(&os) { os.rdbuf(new streambuf(os.rdbuf(), width)); }

fmt::flatten::~flatten() {
  auto buf = static_cast<streambuf *>(_os->rdbuf());
  _os->rdbuf(buf->base());
  delete buf;
}

TEST_CASE("flatten check") {
  test::check([] {
    auto width = gen::range(0, 20);
    std::ostringstream os;
    fmt::flatten flatten(os, width);

    auto size = gen::weight::current;
    while (gen::next(1, size--)) {
      auto indent = std::string(gen::range(0, width * 2), ' ');
      auto contents =
          gen::container<std::string>(gen::range(0, width * 2), [] { return gen::range('a', char('z' + 1)); });
      os << indent << contents << '\n';
    }
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
