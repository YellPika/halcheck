#ifndef HALCHECK_LIB_ITERATOR_FILTER_HPP
#define HALCHECK_LIB_ITERATOR_FILTER_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <memory>

namespace halcheck { namespace lib {

/**
 * @brief An iterator adaptor that skips elements that do not satisfy user-defined predicate.
 * @tparam I The base iterator type.
 * @tparam F The type of predicate.
 * @ingroup lib-iterator
 */
template<typename I, typename F>
class filter_iterator : public lib::iterator_interface<filter_iterator<I, F>> {
public:
  static_assert(lib::is_input_iterator<I>(), "I must be an input iterator");
  static_assert(std::is_copy_constructible<F>(), "F must be copy constructible");
  static_assert(std::is_object<F>(), "F must be an object");
  static_assert(
      lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::iter_reference_t<I>>>(),
      "F must be invocable and return a boolean-testable value");

  template<typename, typename>
  friend class filter_iterator;

  using lib::iterator_interface<filter_iterator>::operator++;
  using lib::iterator_interface<filter_iterator>::operator--;

  /**
   * @brief The type of value pointed to by this type of iterator.
   */
  using value_type = lib::iter_value_t<I>;

  /**
   * @brief The return type of `operator*`.
   */
  using reference = lib::iter_reference_t<I>;

  /**
   * @brief This iterator type does not support `operator->`.
   */
  using pointer = void;

  /**
   * @brief The return type of `operator-`.
   */
  using difference_type = lib::iter_difference_t<I>;

  /**
   * @brief Indicates the level of supported iterator operations this type provides.
   */
  using iterator_category =
      lib::conditional_t<lib::is_random_access_iterator<I>{}, std::bidirectional_iterator_tag, lib::iter_category_t<I>>;

  /**
   * @brief Constructs a default @ref filter_iterator.
   */
  constexpr filter_iterator() = default;

  /**
   * @brief Constructs a @ref filter_iterator from a (begin, end] pair of iterators and a predicate.
   * @param base The base iterator, which determines what the constructed @ref filter_iterator points at.
   * @param end The end iterator, which indicates where there are no further elements.
   * @param fun The predicate.
   */
  filter_iterator(I base, I end, F fun) : _base(std::move(base)), _end(std::move(end)), _func(std::move(fun)) {
    while (_base != _end && !lib::invoke(*_func, *_base))
      ++_base;
  }

  /**
   * @brief Performs a conversion on a @ref filter_iterator with compatible type parameters.
   * @tparam J The type of base iterator to convert from.
   * @tparam G The type of predicate to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_convertible<J, I>()` and
   * `std::is_convertible<G, F>()` hold.
   */
  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_convertible<I2, I>()),
      HALCHECK_REQUIRE(std::is_convertible<F2, F>())>
  constexpr filter_iterator(filter_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _end(std::move(other._end)), _func(std::move(*other._func)) {}

  /**
   * @brief Performs a conversion on a @ref filter_iterator with compatible type parameters.
   * @tparam J The type of base iterator to convert from.
   * @tparam G The type of predicate to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_constructible<I, J>()` and
   * `std::is_constructible<F, G>()` hold, but one of `std::is_convertible<J, I>()` or `std::is_convertible<G, F>()`
   * does not.
   */
  template<
      typename I2,
      typename F2,
      HALCHECK_REQUIRE(std::is_constructible<I, I2>()),
      HALCHECK_REQUIRE(std::is_constructible<F, F2>()),
      HALCHECK_REQUIRE(!std::is_convertible<I2, I>() || !std::is_convertible<F2, F>())>
  explicit constexpr filter_iterator(filter_iterator<I2, F2> other) // NOLINT
      : _base(std::move(other._base)), _end(std::move(other._end)), _func(*std::move(other._func)) {}

  /**
   * @brief Get a reference to the base iterator of this @ref filter_iterator.
   * @return A reference to the base iterator.
   */
  constexpr const I &base() const & noexcept { return _base; }

  /**
   * @brief Get the base iterator of this @ref filter_iterator.
   * @return The base iterator.
   */
  I base() && { return std::move(_base); }

  /**
   * @brief Dereferences the base iterator.
   * @return The result of dereferencing the base iterator.
   */
  constexpr reference operator*() const noexcept(noexcept(*std::declval<const I &>())) { return *_base; }

  /**
   * @brief Advances the base iterator to the next element that satisfies the predicate.
   * @return A reference to `this`.
   */
  filter_iterator &operator++() {
    do {
      ++_base;
    } while (_base != _end && !lib::invoke(*_func, *_base));
    return *this;
  }

  /**
   * @brief Advances the base iterator backwards to the previous element that satisfies the predicate.
   * @return A reference to `this`.
   */
  template<bool _ = true, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<I>() && _)>
  filter_iterator &operator--() {
    do {
      --_base;
    } while (!lib::invoke(*_func, *_base));
    return *this;
  }

private:
  /**
   * @brief Compares two @ref filter_iterator "filter_iterator"s for equality.
   * @param lhs, rhs The iterators to compare.
   * @return The result of `lhs.base() == rhs.base()`.
   */
  friend constexpr bool operator==(const filter_iterator &lhs, const filter_iterator &rhs) {
    return lhs._base == rhs._base;
  }

  I _base, _end;
  lib::optional<F> _func;
};

/**
 * @brief Constructs a @ref filter_iterator.
 * @par Signature
 * @code
 *   template<typename I, typename F>
 *   lib::filter_iterator<I, F> make_filter_iterator(I base, I end, F func)
 * @endcode
 * @tparam I The base iterator type.
 * @tparam F The type of predicate.
 * @param base The base iterator, which determines what the constructed @ref filter_iterator points at.
 * @param end The end iterator, which indicates where there are no further elements.
 * @param fun The predicate.
 * @return The constructed @ref filter_iterator.
 * @ingroup lib-iterator
 */
static const struct {
  template<typename I, typename F>
  lib::filter_iterator<I, F> operator()(I base, I end, F func) const {
    return lib::filter_iterator<I, F>(std::move(base), std::move(end), std::move(func));
  }
} make_filter_iterator;

template<
    typename V,
    typename F,
    HALCHECK_REQUIRE(lib::is_input_range<V>()),
    HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
    HALCHECK_REQUIRE(std::is_object<F>()),
    HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<V>>>())>
class filter_view : public lib::view_interface<filter_view<V, F>> {
private:
  struct ref {
    explicit ref(const F *base = nullptr) : base(base) {}
    template<typename... Args>
    lib::invoke_result_t<const F &, Args...> operator()(Args &&...args) const {
      return lib::invoke(*base, std::forward<Args>(args)...);
    }
    const F *base;
  };

public:
  filter_view() = default;

  filter_view(V base, F func) : _base(std::move(base)), _func(std::move(func)) {}

  template<bool _ = true, HALCHECK_REQUIRE(std::is_copy_constructible<V>() && _)>
  constexpr V base() const & {
    return _base;
  }

  V base() && { return std::move(_base); }

  lib::filter_iterator<lib::iterator_t<V>, ref> begin() {
    return lib::make_filter_iterator(lib::begin(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_input_range<const U>()),
      HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<const U>>>())>
  lib::filter_iterator<lib::iterator_t<const U>, ref> begin() const {
    return lib::make_filter_iterator(lib::begin(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  lib::filter_iterator<lib::iterator_t<V>, ref> end() {
    return lib::make_filter_iterator(lib::end(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_boolean_testable<lib::invoke_result_t<const F &, lib::range_reference_t<const U>>>())>
  lib::filter_iterator<lib::iterator_t<const U>, ref> end() const {
    return lib::make_filter_iterator(lib::end(_base), lib::end(_base), ref{std::addressof(*_func)});
  }

private:
  V _base;
  lib::optional<F> _func;
};

template<typename V, typename F>
lib::filter_view<V, F> filter(V base, F func) {
  return lib::filter_view<V, F>(std::move(base), std::move(func));
}

}} // namespace halcheck::lib

#endif
