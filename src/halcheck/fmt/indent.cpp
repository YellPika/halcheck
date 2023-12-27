#include "halcheck/fmt/indent.hpp"

#include <string>

using namespace halcheck;

namespace {
class streambuf : public std::streambuf {
public:
  explicit streambuf(std::ostream &os, std::size_t offset) {
    _base = os.rdbuf();
    _offset = offset;
  }

  int overflow(int ch) override {
    if (ch == traits_type::eof())
      return sync();

    if (ch != '\n')
      return _base->sputc(ch);

    std::string indent;
    indent.reserve(_offset + 1);
    indent.push_back(ch);
    indent.append(_offset, ' ');
    return _base->sputn(indent.data(), indent.size());
  }

  int sync() override { return _base->pubsync(); }

  streambuf *setbuf(char_type *s, std::streamsize n) override {
    _base->pubsetbuf(s, n);
    return this;
  }

  std::streambuf *base() const { return _base; }

private:
  std::streambuf *_base;
  std::size_t _offset;
};
} // namespace

fmt::indent::indent(std::ostream &os, std::size_t offset) : _os(&os) { os.rdbuf(new streambuf(os, offset)); }

fmt::indent::~indent() {
  auto buf = static_cast<streambuf *>(_os->rdbuf());
  _os->rdbuf(buf->base());
  delete buf;
}
