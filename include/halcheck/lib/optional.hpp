#ifndef HALCHECK_LIB_OPTIONAL_HPP
#define HALCHECK_LIB_OPTIONAL_HPP

#if __cplusplus >= 201606L
#include <optional>
#define HALCHECK_OPTIONAL_NS ::std
#else
#include <tl/optional.hpp>
#define HALCHECK_OPTIONAL_NS ::tl
#endif

namespace halcheck { namespace lib {

using HALCHECK_OPTIONAL_NS::nullopt;
using HALCHECK_OPTIONAL_NS::nullopt_t;
using HALCHECK_OPTIONAL_NS::optional;

}} // namespace halcheck::lib

#endif
