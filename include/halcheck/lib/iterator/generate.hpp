#ifndef HALCHECK_LIB_ITERATOR_GENERATE_HPP
#define HALCHECK_LIB_ITERATOR_GENERATE_HPP

// IWYU pragma: private, include <halcheck/lib/iterator.hpp>

#include <halcheck/lib/functional.hpp>
#include <halcheck/lib/iterator/interface.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>

#include <cstddef>
#include <iterator>

namespace halcheck { namespace lib {

template<
    typename F,
    HALCHECK_REQUIRE(lib::is_copyable<F>()),
    typename = decltype(bool(std::declval<lib::invoke_result_t<F &>>())),
    typename = decltype(*std::declval<lib::invoke_result_t<F &>>())>
class generate_iterator : public lib::iterator_interface<generate_iterator<F>> {
public:
  using lib::iterator_interface<generate_iterator>::operator++;

  using value_type = lib::decay_t<decltype(*std::declval<lib::invoke_result_t<F>>())>;
  using reference = const value_type &;
  using pointer = void;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::input_iterator_tag;

  constexpr generate_iterator() = default;

  explicit generate_iterator(F func) : _func(std::move(func)) { ++(*this); }

  constexpr reference &operator*() const noexcept { return *_value; }

  generate_iterator &operator++() {
    if (auto value = lib::invoke(*_func))
      _value.emplace(std::move(*value));
    else
      _value.reset();
    return *this;
  }

private:
  friend bool operator==(const generate_iterator &lhs, const generate_iterator &rhs) {
    return !lhs._value && !rhs._value;
  }

  lib::optional<F> _func;
  lib::optional<value_type> _value;
};

template<typename F>
lib::generate_iterator<F> make_generate_iterator(F func) {
  return lib::generate_iterator<F>(std::move(func));
}

}} // namespace halcheck::lib
#endif
