#ifndef HALCHECK_GLOG_FILTER_HPP
#define HALCHECK_GLOG_FILTER_HPP

#include <halcheck/gen/discard.hpp>
#include <halcheck/lib/functional.hpp>
#include <halcheck/test/strategy.hpp>

#include <glog/logging.h>

namespace halcheck { namespace glog {

test::strategy filter();

}} // namespace halcheck::glog

#endif
