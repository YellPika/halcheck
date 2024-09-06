#ifndef HALCHECK_CLANG_FUZZER_HPP
#define HALCHECK_CLANG_FUZZER_HPP

#if __clang__
#include <halcheck/test/strategy.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace halcheck { namespace clang {

test::strategy fuzz(
    std::uintmax_t max_size = 100,
    std::uintmax_t max_length = 128,
    const std::vector<std::string> &args = {"-rss_limit_mb=0", "-len_control=0", "-reduce-inputs=0", "-runs=1000"});

}} // namespace halcheck::clang
#endif

#endif
