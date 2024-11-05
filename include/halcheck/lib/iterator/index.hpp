#ifndef HALCHECK_LIB_ITERATOR_INDEX_HPP
#define HALCHECK_LIB_ITERATOR_INDEX_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>

namespace halcheck { namespace lib {

/**
 * @brief An iterator into a @ref lib::is_random_access_range "random-access range" that is invalidated if and only if
 * the index of the pointed-to element changes.
 * @tparam R The type of range to point into.
 * @ingroup lib-iterator
 */
template<typename R>
class index_iterator : public lib::iterator_interface<index_iterator<R>> {
public:
  static_assert(lib::is_random_access_range<R>(), "R should be a random access range");

  template<typename>
  friend class index_iterator;

  using lib::iterator_interface<index_iterator>::operator++;
  using lib::iterator_interface<index_iterator>::operator--;
  using lib::iterator_interface<index_iterator>::operator-=;
  using lib::iterator_interface<index_iterator>::operator[];

  /**
   * @brief The type of value this iterator points to.
   */
  using value_type = lib::range_value_t<R>;

  /**
   * @brief The type of value returned by `operator*`.
   */
  using reference = lib::range_reference_t<R>;

  /**
   * @brief This iterator does not support `operator->`.
   */
  using pointer = void;

  /**
   * @brief The type of distances between iterators.
   */
  using difference_type = lib::range_difference_t<R>;

  /**
   * @brief Indicates that @ref index_iterator is a @ref lib::is_random_access_iterator "random-access iterator".
   */
  using iterator_category = std::random_access_iterator_tag;

  /**
   * @brief Constructs a default @ref index_iterator.
   */
  index_iterator() = default;

  /**
   * @brief Constructs an @ref index_iterator pointing into the given range.
   * @param base The range to point into.
   * @param index The offset within the range to point at.
   */
  explicit index_iterator(R &base, difference_type index = 0) : _base(std::addressof(base)), _index(index) {}

  /**
   * @brief Converts between @ref index_iterator "index_iterator"s.
   * @tparam T The type of range to convert from.
   * @param other The iterator to convert from.
   * @details This overload only participates in overload resolution if `std::is_convertible<T *, R *>()` holds.
   */
  template<typename T, HALCHECK_REQUIRE(std::is_convertible<T *, R *>())>
  index_iterator(const index_iterator<T> &other) // NOLINT: implicit conversion
      : _base(other._base), _index(other._index) {}

  /**
   * @brief Obtains the element this iterator points to.
   * @return A reference to the pointed-to element.
   */
  reference operator*() const { return lib::begin(*_base)[_index]; }

  /**
   * @brief Advances this iterator.
   * @return A reference to `this`.
   */
  index_iterator &operator++() {
    ++_index;
    return *this;
  }

  /**
   * @brief Advances this iterator backwards.
   * @return A reference to `this`.
   */
  index_iterator &operator--() {
    --_index;
    return *this;
  }

  /**
   * @brief Advances this iterator by the specified number of steps.
   * @return A reference to `this`.
   */
  index_iterator &operator+=(difference_type n) {
    _index += n;
    return *this;
  }

  /**
   * @brief Gets the index of the element this iterator points to.
   * @return The index of the element this iterator points to.
   */
  difference_type index() const { return _index; }

private:
  /**
   * @brief Determines if two iterators are equal.
   * @param lhs, rhs The iterators to compare.
   * @return `lhs.index() == rhs.index()`
   */
  friend bool operator==(const index_iterator &lhs, const index_iterator &rhs) { return lhs._index == rhs._index; }

  /**
   * @brief Computes the distance between two iterators.
   * @param lhs, rhs The iterators to compare.
   * @return `lhs.index() - rhs.index()`
   */
  friend difference_type operator-(const index_iterator &lhs, const index_iterator &rhs) {
    return lhs._index - rhs._index;
  }

  R *_base = nullptr;
  difference_type _index = 0;
};

/**
 * @brief Constructs an @ref index_iterator.
 * @par Signature
 * @code
 *   template<typename R>
 *   lib::index_iterator<R> make_index_iterator(R &base, lib::range_difference_t<R> index = 0);
 * @endcode
 * @tparam R The type of range to point into.
 * @param base The range to point into.
 * @param index The offset within the range to point at.
 * @ingroup lib-iterator
 */
static const struct {
  template<typename R>
  lib::index_iterator<R> operator()(R &base, lib::range_difference_t<R> index = 0) {
    return lib::index_iterator<R>(base, std::move(index));
  }
} make_index_iterator;

}} // namespace halcheck::lib

#endif
