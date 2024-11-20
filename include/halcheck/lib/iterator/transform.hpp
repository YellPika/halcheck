#ifndef HALCHECK_LIB_ITERATOR_TRANSFORM_HPP
#define HALCHECK_LIB_ITERATOR_TRANSFORM_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/base.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/iterator/type_traits.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/pp.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <iterator>
#include <memory>
#include <type_traits>

namespace halcheck { namespace lib {

/**
 * @brief A transforming iterator adaptor.
 * @tparam I The base iterator type.
 * @tparam F The type of function to apply to each enumerated value of @p I.
 * @ingroup lib-iterator
 */
template<typename I, typename F>
class transform_iterator : public lib::iterator_interface<transform_iterator<I, F>> {
private:
  static_assert(lib::is_input_iterator<I>(), "I must be an input iterator");
  static_assert(std::is_copy_constructible<F>(), "F must be copy constructible");
  static_assert(std::is_object<F>(), "F must an object type");
  static_assert(lib::is_invocable<const F &, lib::iter_reference_t<I>>(), "F must be invocable");

  template<typename, typename>
  friend class transform_iterator;

public:
  using lib::iterator_interface<transform_iterator>::operator++;
  using lib::iterator_interface<transform_iterator>::operator--;
  using lib::iterator_interface<transform_iterator>::operator-=;
  using lib::iterator_interface<transform_iterator>::operator[];

  /**
   * @brief The type returned by `operator*`.
   */
  using reference = lib::invoke_result_t<const F &, lib::iter_reference_t<I>>;

  /**
   * @brief Indicates the level of supported iterator operations this type provides.
   */
  using iterator_category =
      lib::conditional_t<!std::is_reference<reference>(), std::input_iterator_tag, lib::iter_category_t<I>>;

  /**
   * @brief The type of value pointed to by this iterator.
   */
  using value_type = lib::remove_cvref_t<lib::invoke_result_t<const F &, lib::iter_reference_t<I>>>;

  /**
   * @brief The type returned by `operator-` when given two @ref transform_iterator "transform_iterator"s.
   */
  using difference_type = lib::iter_difference_t<I>;

  /**
   * @brief This type does not support `operator->`.
   */
  using pointer = void;

  /**
   * @brief Constructs a default @ref transform_iterator.
   */
  constexpr transform_iterator() = default;

  /**
   * @brief Constructs a @ref transform_iterator with the given base iterator and function.
   * @param current The base iterator.
   * @param fun The function to apply to each element.
   */
  constexpr transform_iterator(I current, F fun) : _base(std::move(current)), _func(std::move(fun)) {}

  /**
   * @brief Performs a conversion on a @ref transform_iterator with compatible type parameters.
   * @tparam J The type of base iterator to convert from.
   * @tparam G The type of function to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_convertible<J, I>()` and
   * `std::is_convertible<G, F>()` hold.
   */
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE(std::is_convertible<J, I>()),
      HALCHECK_REQUIRE(std::is_convertible<G, F>())>
  constexpr transform_iterator(transform_iterator<J, G> other) // NOLINT
      : _base(std::move(other._base)), _func(std::move(*other._func)) {}

  /**
   * @brief Performs a conversion on a @ref transform_iterator with compatible type parameters.
   * @tparam J The type of base iterator to convert from.
   * @tparam G The type of function to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_constructible<I, J>()` and
   * `std::is_constructible<F, G>()` hold, but one of `std::is_convertible<J, I>()` or `std::is_convertible<G, F>()`
   * does not.
   */
  template<
      typename J,
      typename G,
      HALCHECK_REQUIRE(std::is_constructible<I, J>()),
      HALCHECK_REQUIRE(std::is_constructible<F, G>()),
      HALCHECK_REQUIRE(!std::is_convertible<J, I>() || !std::is_convertible<G, F>())>
  explicit constexpr transform_iterator(transform_iterator<J, G> other) // NOLINT
      : _base(std::move(other._base)), _func(*std::move(other._func)) {}

  /**
   * @brief Get a reference to the base iterator of this @ref transform_iterator.
   * @return A reference to the base iterator.
   */
  constexpr const I &base() const & noexcept { return _base; }

  /**
   * @brief Get the base iterator of this @ref transform_iterator.
   * @return The base iterator.
   */
  I base() && { return std::move(_base); }

  /**
   * @brief Dereferences the base iterator an applies the function.
   * @return The result of applying the function.
   */
  constexpr lib::invoke_result_t<const F &, lib::iter_reference_t<I>> operator*() const
      noexcept(lib::is_nothrow_invocable<const F &, lib::iter_reference_t<I>>()) {
    return lib::invoke(*_func, *_base);
  }

  /**
   * @brief Advances the base iterator.
   * @return A reference to `this`.
   */
  transform_iterator &operator++() {
    ++_base;
    return *this;
  }

  /**
   * @brief Advances the base iterator one step backwards.
   * @return A reference to `this`.
   */
  template<bool _ = true, HALCHECK_REQUIRE(lib::is_bidirectional_iterator<I>() && _)>
  transform_iterator &operator--() {
    --_base;
    return *this;
  }

  /**
   * @brief Advances the base iterator the specified amount.
   * @param n The amount to advance the base iterator.
   * @return A reference to `this`.
   */
  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  transform_iterator &operator+=(difference_type n) {
    _base += n;
    return *this;
  }

private:
  /**
   * @brief Compares two @ref transform_iterator "transform_iterator"s for equality.
   * @param lhs, rhs The iterators to compare.
   * @return The result of `lhs.base() == rhs.base()`.
   */
  friend constexpr bool operator==(const transform_iterator &lhs, const transform_iterator &rhs) {
    return lhs._base == rhs._base;
  }

  /**
   * @brief Computes the difference between two iterators.
   * @param lhs, rhs The iterators to compare.
   * @return The result of `lhs.base() - rhs.base()`.
   */
  template<bool _ = true, HALCHECK_REQUIRE(lib::is_random_access_iterator<I>() && _)>
  friend difference_type operator-(const transform_iterator &lhs, const transform_iterator &rhs) {
    return lhs._base - rhs._base;
  }

  I _base;
  lib::optional<F> _func;
};

/**
 * @brief Constructs a @ref transform_iterator.
 * @par Signature
 * @code
 *   template<typename I, typename F>
 *   lib::transform_iterator<I, F> make_transform_iterator(I base, F func)
 * @endcode
 * @tparam I The type of base iterator.
 * @tparam F the type of function.
 * @param base The base iterator.
 * @param func The function.
 * @return `lib::transform_iterator<I, F>(std::move(base), std::move(func))`
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
  template<
      typename I,
      typename F,
      HALCHECK_REQUIRE(lib::is_input_iterator<I>()),
      HALCHECK_REQUIRE(std::is_copy_constructible<F>()),
      HALCHECK_REQUIRE(std::is_object<F>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::iter_reference_t<I>>())>
  lib::transform_iterator<I, F> operator()(I base, F func) const {
    return lib::transform_iterator<I, F>(std::move(base), std::move(func));
  }
} make_transform_iterator;

/**
 * @brief A transforming iterator adaptor.
 * @tparam V The base range type.
 * @tparam F The type of function to apply to the values contained in a @p V.
 * @ingroup lib-iterator
 */
template<typename V, typename F>
class transform_view : public lib::view_interface<transform_view<V, F>> {
private:
  static_assert(lib::is_input_range<V>(), "V must be an input range");
  static_assert(std::is_move_constructible<F>(), "F must be move constructible");
  static_assert(std::is_object<F>(), "F must be an object");
  static_assert(lib::is_invocable<const F &, lib::range_reference_t<V>>(), "F must be invocable");

  template<typename, typename>
  friend class transform_view;

  struct ref {
    explicit ref(const F *base = nullptr) : base(base) {}
    template<typename... Args>
    lib::invoke_result_t<const F &, Args...> operator()(Args &&...args) const {
      return lib::invoke(*base, std::forward<Args>(args)...);
    }
    const F *base;
  };

public:
  /**
   * @brief Constructs a default @ref transform_view.
   */
  constexpr transform_view() = default;

  /**
   * @brief Constructs a @ref transform_view from the given base range and function.
   * @param base The base range.
   * @param func The function to apply to elements of the base range.
   */
  constexpr transform_view(V base, F func) : _base(std::move(base)), _func(std::move(func)) {}

  /**
   * @brief Performs a conversion on a @ref transform_view with compatible type parameters.
   * @tparam J The type of base range to convert from.
   * @tparam G The type of function to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_convertible<V2, V>()` and
   * `std::is_convertible<G, F>()` hold.
   */
  template<
      typename V2,
      typename G,
      HALCHECK_REQUIRE(std::is_convertible<V2, V>()),
      HALCHECK_REQUIRE(std::is_convertible<G, F>())>
  constexpr transform_view(transform_view<V2, G> other) // NOLINT
      : _base(std::move(other._base)), _func(std::move(*other._func)) {}

  /**
   * @brief Performs a conversion on a @ref transform_view with compatible type parameters.
   * @tparam J The type of base range to convert from.
   * @tparam G The type of function to convert from.
   * @param other The iterator to copy.
   * @details This overload only participates in overload resolution if `std::is_constructible<V, V2>()` and
   * `std::is_constructible<F, G>()` hold, but one of `std::is_convertible<J, V>()` or `std::is_convertible<G, F>()`
   * does not.
   */
  template<
      typename V2,
      typename G,
      HALCHECK_REQUIRE(std::is_constructible<V, V2>()),
      HALCHECK_REQUIRE(std::is_constructible<F, G>()),
      HALCHECK_REQUIRE(!std::is_convertible<V2, V>() || !std::is_convertible<G, F>())>
  explicit constexpr transform_view(transform_view<V2, G> other) // NOLINT
      : _base(std::move(other._base)), _func(*std::move(other._func)) {}

  /**
   * @brief Gets copy of the base range of this view.
   * @return The base range of this view.
   * @details This overload only participates in overload resolution if `std::is_copy_constructible<V>()` is satisfied.
   */
  template<bool _ = true, HALCHECK_REQUIRE(std::is_copy_constructible<V>() && _)>
  constexpr V base() const & {
    return _base;
  }

  /**
   * @brief Gets the base range of this view.
   * @return The base range of this view.
   */
  V base() && { return std::move(_base); }

  /**
   * @brief Gets an iterator pointing to the beginning of this view.
   * @return An iterator pointning to the beginning of this view.
   */
  lib::transform_iterator<lib::iterator_t<V>, ref> begin() {
    return lib::make_transform_iterator(lib::begin(_base), ref{std::addressof(*_func)});
  }

  /**
   * @brief Gets an iterator pointing to the beginning of this view.
   * @return An iterator pointning to the beginning of this view.
   */
  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<const U>>())>
  lib::transform_iterator<lib::iterator_t<const U>, ref> begin() const {
    return lib::make_transform_iterator(lib::begin(_base), ref{std::addressof(*_func)});
  }

  /**
   * @brief Gets an iterator pointing past the end of this view.
   * @return An iterator pointning past the end of this view.
   */
  lib::transform_iterator<lib::iterator_t<V>, ref> end() {
    return lib::make_transform_iterator(lib::end(_base), ref{std::addressof(*_func)});
  }

  /**
   * @brief Gets an iterator pointing past the end of this view.
   * @return An iterator pointning past the end of this view.
   */
  template<
      typename U = V,
      HALCHECK_REQUIRE(lib::is_range<const U>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<const U>>())>
  lib::transform_iterator<lib::iterator_t<const U>, ref> end() const {
    return lib::make_transform_iterator(lib::end(_base), ref{std::addressof(*_func)});
  }

  /**
   * @brief Computes the number of elements in this view.
   * @return The number of elements in this view.
   */
  template<typename U = V, HALCHECK_REQUIRE(lib::is_sized_range<U>())>
  lib::range_difference_t<U> size() {
    return lib::size(_base);
  }

  /**
   * @brief Computes the number of elements in this view.
   * @return The number of elements in this view.
   */
  template<typename U = V, HALCHECK_REQUIRE(lib::is_sized_range<const U>())>
  lib::range_difference_t<const U> size() const {
    return lib::size(_base);
  }

private:
  V _base;
  lib::optional<F> _func;
};

/**
 * @brief Constructs a @ref transform_view.
 * @par Signature
 * @code
 *   template<typename R, typename F>
 *   lib::transform_view<R, F> make_transform_view(R base, F func)
 * @endcode
 * @tparam R The type of base range.
 * @tparam F the type of function.
 * @param base The base range.
 * @param func The function.
 * @return `lib::transform_view<R, F>(std::move(base), std::move(func))`
 * @ingroup lib-iterator
 */
HALCHECK_INLINE_CONSTEXPR struct {
  template<
      typename V,
      typename F,
      HALCHECK_REQUIRE(lib::is_input_range<V>()),
      HALCHECK_REQUIRE(std::is_move_constructible<F>()),
      HALCHECK_REQUIRE(std::is_object<F>()),
      HALCHECK_REQUIRE(lib::is_invocable<const F &, lib::range_reference_t<V>>())>
  lib::transform_view<V, F> operator()(V base, F func) const {
    return lib::transform_view<V, F>(std::move(base), std::move(func));
  }
} transform;

}} // namespace halcheck::lib

#endif
