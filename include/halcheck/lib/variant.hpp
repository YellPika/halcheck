#ifndef HALCHECK_LIB_VARIANT_HPP
#define HALCHECK_LIB_VARIANT_HPP

#if __cplusplus >= 201606L
#include <variant> // IWYU pragma: export
#define HALCHECK_VARIANT_NS ::std
#else
#include <mpark/in_place.hpp> // IWYU pragma: export
#include <mpark/variant.hpp>  // IWYU pragma: export
#define HALCHECK_VARIANT_NS ::mpark
#endif

namespace halcheck { namespace lib {

using HALCHECK_VARIANT_NS::get;
using HALCHECK_VARIANT_NS::get_if;
using HALCHECK_VARIANT_NS::holds_alternative;
using HALCHECK_VARIANT_NS::in_place_index_t;
using HALCHECK_VARIANT_NS::in_place_type_t;
using HALCHECK_VARIANT_NS::monostate;
using HALCHECK_VARIANT_NS::variant;
using HALCHECK_VARIANT_NS::visit;

}} // namespace halcheck::lib

#endif
