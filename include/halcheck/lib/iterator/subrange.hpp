#ifndef HALCHECK_LIB_ITERATOR_SUBRANGE_HPP
#define HALCHECK_LIB_ITERATOR_SUBRANGE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/numeric.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <type_traits>

namespace halcheck { namespace lib {

template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>())>
class subrange : public lib::view_interface<subrange<I>> {
public:
  constexpr subrange() = default;

  constexpr subrange(I begin, I end) : _begin(std::move(begin)), _end(std::move(end)) {}

  constexpr I begin() const { return _begin; }

  constexpr I end() const { return _end; }

private:
  I _begin;
  I _end;
};

template<typename I>
lib::subrange<I> make_subrange(I begin, I end) {
  return lib::subrange<I>(std::move(begin), std::move(end));
}

template<typename I>
struct enable_borrowed_range<lib::subrange<I>> : std::true_type {};

}} // namespace halcheck::lib

#endif
