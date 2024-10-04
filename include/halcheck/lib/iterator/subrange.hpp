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

enum class subrange_kind : bool { unsized, sized };

template<
    typename I,
    typename S = I,
    lib::subrange_kind K = lib::is_sized_sentinel_for<S, I>() ? lib::subrange_kind::sized : lib::subrange_kind::unsized,
    typename = lib::to_void<
        lib::input_or_output_iterator<I>,
        lib::sentinel_for<S, I>,
        lib::enable_if_t<K == lib::subrange_kind::sized || !lib::is_sized_sentinel_for<S, I>{}>>>
class subrange : public lib::view_interface<subrange<I, S, K>> {
public:
  constexpr subrange() = default;

  constexpr subrange(I begin, S end) : _begin(std::move(begin)), _end(std::move(end)) {}

  template<bool _ = true, HALCHECK_REQUIRE(!lib::is_copyable<I>() && _)>
  I begin() {
    return std::move(_begin);
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_copyable<I>() && _)>
  constexpr I begin() const {
    return _begin;
  }

  template<bool _ = true, HALCHECK_REQUIRE(!lib::is_copyable<S>() && _)>
  S end() {
    return std::move(_end);
  }

  template<bool _ = true, HALCHECK_REQUIRE(lib::is_copyable<S>() && _)>
  constexpr S end() const {
    return _end;
  }

  constexpr bool empty() const { return _begin == _end; }

  template<bool _ = true, HALCHECK_REQUIRE(K == lib::subrange_kind::sized && _)>
  constexpr lib::make_unsigned_t<lib::iter_difference_t<I>> size() const {
    return lib::to_unsigned(_end - _begin);
  }

private:
  I _begin;
  S _end;
};

template<typename I, typename S>
lib::subrange<I, S> make_subrange(I begin, S end) {
  return lib::subrange<I, S>(std::move(begin), std::move(end));
}

template<typename I, typename S, lib::subrange_kind K>
struct enable_borrowed_range<lib::subrange<I, S, K>> : std::true_type {};

}} // namespace halcheck::lib

#endif
