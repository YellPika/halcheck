#ifndef HALCHECK_FMT_FLATTEN_HPP
#define HALCHECK_FMT_FLATTEN_HPP

#include <cstddef>
#include <ostream>

namespace halcheck { namespace fmt {

/// @brief Intelligently deletes newlines and indentation passed to a
///        std::ostream while keeping line widths within the specified value.
class flatten {
public:
  /// @brief Imbues the given std::ostream with flattening behaviour.
  /// @param os The std::ostream to modify.
  /// @param width The maximum width allowed per line.
  explicit flatten(std::ostream &os, std::size_t width = 80);
  flatten(const flatten &) = delete;
  flatten &operator=(const flatten &) = delete;
  ~flatten();

private:
  std::ostream *_os;
};

}} // namespace halcheck::fmt

#endif
