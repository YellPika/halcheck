#ifndef HALCHECK_LIB_ITERATOR_SUBRANGE_HPP
#define HALCHECK_LIB_ITERATOR_SUBRANGE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <type_traits>

namespace halcheck { namespace lib {

/**
 * @brief Describes a range formed from a pair of iterators.
 * @tparam I The type of iterator contained in the @ref subrange.
 * @ingroup lib-iterator
 */
template<typename I>
class subrange : public lib::view_interface<subrange<I>> {
public:
  static_assert(lib::is_iterator<I>(), "I must be an iterator");

  /**
   * @brief Constructs an empty @ref subrange.
   * @details This overload participates in overload resolution only if `std::is_default_constructible<I>()` holds.
   */
  constexpr subrange() = default;

  /**
   * @brief Constructs a @ref subrange from a pair of iterators.
   * @param first An iterator pointing at the first element in the @ref subrange.
   * @param last An iterator pointing past the last element of the @ref subrange.
   */
  constexpr subrange(I first, I last) : _begin(std::move(first)), _end(std::move(last)) {}

  /**
   * @brief Gets an iterator pointing at the first element in the @ref subrange.
   * @return An iterator pointing at the first element in the @ref subrange.
   */
  constexpr I begin() const { return _begin; }

  /**
   * @brief Gets an iterator pointing past the last element in the @ref subrange.
   * @return An iterator pointing past the last element in the @ref subrange.
   */
  constexpr I end() const { return _end; }

private:
  I _begin;
  I _end;
};

/**
 * @brief Constructs a @ref subrange from a pair of iterators.
 * @tparam I The type of iterator contained in the @ref subrange.
 * @param first An iterator pointing at the first element in the @ref subrange.
 * @param last An iterator pointing past the last element of the @ref subrange.
 * @return A @ref subrange `x` satisfying `x.begin() == first` and `x.end() == last`.
 * @ingroup lib-iterator
 */
template<typename I, HALCHECK_REQUIRE(lib::is_iterator<I>())>
lib::subrange<I> make_subrange(I first, I last) {
  return lib::subrange<I>(std::move(first), std::move(last));
}

template<typename I>
struct enable_borrowed_range<lib::subrange<I>> : std::true_type {};

}} // namespace halcheck::lib

#endif
