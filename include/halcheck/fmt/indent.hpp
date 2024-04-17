#ifndef HALCHECK_FMT_ALIGN_HPP
#define HALCHECK_FMT_ALIGN_HPP

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <ostream>

namespace halcheck { namespace fmt {

/// @brief Forces all new lines to be indented according to a specified amount.
///        Also modifies tellp so that it reports the current position relative
///        to the indentation level.
class indent {
public:
  /// @brief Indents the contents of the given std::ostream a specified amount.
  /// @param os The std::ostream to modify.
  /// @param offset The amount of spaces to indent.
  indent(std::ostream &os, std::size_t offset = 4);

  /// @brief Indents the contents of the given std::ostream according to the
  ///        given function.
  /// @tparam F The type of function to apply.
  /// @param func The function to apply on every new line.
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, std::ostream &>())>
  indent(std::ostream &os, F func) : _buf(new streambuf<F>(os, std::move(func))) {}

private:
  template<typename F, HALCHECK_REQUIRE(lib::is_invocable<F, std::ostream &>())>
  class streambuf : public std::streambuf {
  public:
    explicit streambuf(std::ostream &os, F func) : _os(&os), _buf(os.rdbuf()), _func(std::move(func)) {
      _os->rdbuf(this);
    }

    ~streambuf() { _os->rdbuf(_buf); }

    int overflow(int ch) override {
      if (ch == traits_type::eof())
        return sync();

      auto output = _buf->sputc(ch);
      if (ch == '\n') {
        std::ostream os(_buf);
        _func(os);
      }
      return output;
    }

    int sync() override { return _buf->pubsync(); }

    streambuf *setbuf(char_type *s, std::streamsize n) override {
      _buf->pubsetbuf(s, n);
      return this;
    }

  private:
    std::ostream *_os;
    std::streambuf *_buf;
    F _func;
  };

  std::unique_ptr<std::streambuf> _buf;
};

}} // namespace halcheck::fmt

#endif
