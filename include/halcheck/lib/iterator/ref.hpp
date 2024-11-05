#ifndef HALCHECK_LIB_ITERATOR_REF_HPP
#define HALCHECK_LIB_ITERATOR_REF_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/iterator/range.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <memory>

namespace halcheck { namespace lib {

// See https://en.cppreference.com/w/cpp/ranges/ref_view

template<typename R>
class ref_view : public lib::view_interface<ref_view<R>> {
private:
  static_assert(std::is_object<R>(), "R must be an object type");
  static_assert(lib::is_range<R>(), "R must be a range type");

  template<typename>
  friend class ref_view;

  static std::true_type FUN(R &);
  static std::false_type FUN(R &&);

public:
  template<
      typename T,
      HALCHECK_REQUIRE(!std::is_same<lib::remove_cvref_t<T>, ref_view>()),
      HALCHECK_REQUIRE(std::is_convertible<T, R &>()),
      HALCHECK_REQUIRE(decltype(FUN(std::declval<T>()))())>
  constexpr ref_view(T &&base) // NOLINT
      : _base(std::addressof(static_cast<R &>(std::forward<T>(base)))) {}

  template<typename T, HALCHECK_REQUIRE(std::is_convertible<T *, R *>())>
  constexpr ref_view(const ref_view<T> &other) // NOLINT
      : _base(other._base) {}

  constexpr R &base() const { return *_base; }

  constexpr lib::iterator_t<R> begin() const { return lib::begin(*_base); }

  constexpr lib::iterator_t<R> end() const { return lib::end(*_base); }

  template<typename T = R, typename = decltype(lib::empty(std::declval<const T &>()))>
  constexpr bool empty() const {
    return lib::empty(*_base);
  }

  template<typename T = R, HALCHECK_REQUIRE(lib::is_sized_range<T>())>
  constexpr bool size() const {
    return lib::size(*_base);
  }

private:
  R *_base;
};

template<typename R>
struct enable_borrowed_range<lib::ref_view<R>> : std::true_type {};

static const struct {
  template<typename R>
  constexpr lib::ref_view<R> operator()(R &range) const {
    return lib::ref_view<R>(range);
  }
} ref;

}} // namespace halcheck::lib

#endif
