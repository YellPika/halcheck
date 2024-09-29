#include "halcheck/lib/iterator.hpp"
#include <halcheck/lib/iterator.hpp>

#include <forward_list>
#include <set>
#include <type_traits>
#include <vector>

using namespace halcheck;

namespace test1 {
using iterator = std::vector<int>::iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(lib::is_random_access_iterator<iterator>(), "");
static_assert(lib::is_contiguous_iterator<iterator>(), "");
} // namespace test1

namespace test2 {
using iterator = std::vector<int>::const_iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(lib::is_random_access_iterator<iterator>(), "");
static_assert(lib::is_contiguous_iterator<iterator>(), "");
} // namespace test2

namespace test3 {
using iterator = std::set<int>::iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(!lib::is_random_access_iterator<iterator>(), "");
static_assert(!lib::is_contiguous_iterator<iterator>(), "");
} // namespace test3

namespace test4 {
using iterator = std::set<int>::const_iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(!lib::is_random_access_iterator<iterator>(), "");
static_assert(!lib::is_contiguous_iterator<iterator>(), "");
} // namespace test4

namespace test5 {
using iterator = std::forward_list<int>::iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(!lib::is_bidirectional_iterator<iterator>(), "");
static_assert(!lib::is_random_access_iterator<iterator>(), "");
static_assert(!lib::is_contiguous_iterator<iterator>(), "");
} // namespace test5

namespace test6 {
using iterator = std::forward_list<int>::const_iterator;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(!lib::is_bidirectional_iterator<iterator>(), "");
static_assert(!lib::is_random_access_iterator<iterator>(), "");
static_assert(!lib::is_contiguous_iterator<iterator>(), "");
} // namespace test6

namespace test7 {
using iterator = lib::index_iterator<std::vector<int>>;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(lib::is_random_access_iterator<iterator>(), "");
static_assert(lib::is_contiguous_iterator<iterator>(), "");
} // namespace test7

namespace test8 {
using iterator = lib::index_iterator<const std::vector<int>>;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(lib::is_bidirectional_iterator<iterator>(), "");
static_assert(lib::is_random_access_iterator<iterator>(), "");
static_assert(lib::is_contiguous_iterator<iterator>(), "");
} // namespace test8

namespace test9 {
using iterator = lib::concat_iterator<std::vector<std::vector<int>>::const_iterator>;
static_assert(lib::is_iterator<iterator>(), "");
static_assert(lib::is_input_iterator<iterator>(), "");
static_assert(std::is_default_constructible<iterator>(), "");
static_assert(lib::is_forward_iterator<iterator>(), "");
static_assert(!lib::is_bidirectional_iterator<iterator>(), "");
static_assert(!lib::is_random_access_iterator<iterator>(), "");
static_assert(!lib::is_contiguous_iterator<iterator>(), "");
} // namespace test9
