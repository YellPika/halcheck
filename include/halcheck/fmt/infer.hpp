#ifndef HALCHECK_FMT_INFER_HPP
#define HALCHECK_FMT_INFER_HPP

#include <ostream>

namespace halcheck { namespace fmt {

/// @brief Determines whether the infer flag is set.
/// @param os The std::ostream to query.
/// @return true if the infer flag is set; false otherwise.
bool infer(std::ostream &os);

/// @brief Sets the infer flag.
/// @param os The std::ostream to modify.
/// @param value true to set the infer flag; false to unset it.
/// @return The previous value of the infer flag.
bool infer(std::ostream &os, bool value);

}} // namespace halcheck::fmt

#endif
