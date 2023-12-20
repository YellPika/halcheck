#ifndef HALCHECK_FMT_ALIGN_HPP
#define HALCHECK_FMT_ALIGN_HPP

#include <cstddef>
#include <ostream>

namespace halcheck { namespace fmt {

/// @brief Forces all new lines to be indented according to a specified amount.
///        Also modifies tellp so that it reports the current position relative
///        to the indentation level.
class indent {
public:
  /// @brief Indents the contents of the given std::ostream a specified amount.
  /// @param os The std::ostream to modify.
  indent(std::ostream &os, std::size_t indent = 4);

  indent(const indent &) = delete;
  indent &operator=(const indent &) = delete;
  ~indent();

private:
  std::ostream *_os;
};

}} // namespace halcheck::fmt

#endif
