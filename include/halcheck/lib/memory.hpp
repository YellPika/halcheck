#ifndef HALCHECK_LIB_MEMORY_HPP
#define HALCHECK_LIB_MEMORY_HPP

#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/variant.hpp>

#include <memory> // IWYU pragma: export
#include <type_traits>

namespace halcheck { namespace lib {

template<typename T, typename... Args, HALCHECK_REQUIRE(!std::is_array<T>())>
std::unique_ptr<T> make_unique(Args... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
class unique_box {
public:
  unique_box(const unique_box &) = delete;
  unique_box &operator=(const unique_box &) = delete;
  ~unique_box() = default;

  template<typename U = T, HALCHECK_REQUIRE(std::is_default_constructible<U>())>
  unique_box() : _impl(new T()) {}

  unique_box(unique_box &&other) noexcept(false) : _impl(new T(std::move(*other))) {
    static_assert(lib::is_movable<T>(), "T must be movable");
  }

  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  explicit unique_box(lib::in_place_t, Args &&...args) : _impl(new T(std::forward<Args>(args)...)) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, unique_box>())>
  unique_box(U &&value) // NOLINT
      : _impl(new T(std::forward<U>(value))) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(!std::is_convertible<U &&, T>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, unique_box>())>
  explicit unique_box(U &&value) : _impl(new T(std::forward<U>(value))) {}

  unique_box &operator=(unique_box &&other) noexcept(false) {
    *_impl = std::move(*other);
    return *this;
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_assignable<T &, U>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, unique_box>())>
  unique_box &operator=(U &&value) {
    *_impl = value;
    return *this;
  }

  T &operator*() { return *get(); }
  const T &operator*() const { return *get(); }

  T *operator->() { return get(); }
  const T *operator->() const { return get(); }

  T *get() { return _impl.get(); }
  const T *get() const { return _impl.get(); }

  void swap(unique_box &other) {
    using std::swap;
    swap(*get(), *other);
  }

  operator T &() { return *_impl; }             // NOLINT
  operator const T &() const { return *_impl; } // NOLINT

private:
  std::unique_ptr<T> _impl;
};

template<typename T>
class box : private unique_box<T> {
public:
  using unique_box<T>::operator*;
  using unique_box<T>::operator->;
  using unique_box<T>::get;
  using unique_box<T>::swap;
  using unique_box<T>::operator T &;
  using unique_box<T>::operator const T &;

  box(box &&) noexcept(false) = default;
  box &operator=(box &&) noexcept(false) = default;
  ~box() = default;

  template<typename U = T, HALCHECK_REQUIRE(std::is_default_constructible<U>())>
  box() {}

  box(const box &other) : unique_box<T>(*other) { static_assert(lib::is_copyable<T>(), "T must be copyable"); }

  template<typename... Args, HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
  explicit box(lib::in_place_t tag, Args &&...args) : unique_box<T>(tag, std::forward<Args>(args)...) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_convertible<U &&, T>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, box>())>
  box(U &&value) // NOLINT
      : unique_box<T>(std::forward<U>(value)) {}

  template<
      typename U = T,
      HALCHECK_REQUIRE(!std::is_convertible<U &&, T>()),
      HALCHECK_REQUIRE(std::is_constructible<T, U &&>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, box>())>
  explicit box(U &&value) : unique_box<T>(std::forward<U>(value)) {}

  box &operator=(const box &other) { // NOLINT
    unique_box<T>::operator=(*other);
    return *this;
  }

  template<
      typename U = T,
      HALCHECK_REQUIRE(std::is_assignable<T &, U>()),
      HALCHECK_REQUIRE(!std::is_same<lib::decay_t<U>, box>())>
  box &operator=(U &&value) {
    *get() = value;
    return *this;
  }
};

template<typename T>
struct default_copy {
  T *operator()(const T &value) const { return new T(value); }
};

template<typename T, typename Copier = lib::default_copy<T>, typename Deleter = std::default_delete<T>>
class cloned_ptr : private std::unique_ptr<T, Deleter> {
public:
  using std::unique_ptr<T, Deleter>::operator*;
  using std::unique_ptr<T, Deleter>::operator->;
  using std::unique_ptr<T, Deleter>::get;
  using std::unique_ptr<T, Deleter>::swap;
  using std::unique_ptr<T, Deleter>::get_deleter;
  using std::unique_ptr<T, Deleter>::release;
  using std::unique_ptr<T, Deleter>::reset;
  using std::unique_ptr<T, Deleter>::operator bool;

  cloned_ptr() = default;
  cloned_ptr(cloned_ptr &&) = default;
  cloned_ptr &operator=(cloned_ptr &&) = default;
  ~cloned_ptr() = default;

  cloned_ptr(const cloned_ptr &other)
      : std::unique_ptr<T, Deleter>(other._copier(other.get()), other.get_deleter()), _copier(other._copier) {}

  explicit cloned_ptr(T *value, Copier copier = Copier(), Deleter deleter = Deleter()) noexcept
      : std::unique_ptr<T, Deleter>(value, std::move(deleter)), _copier(std::move(copier)) {}

  cloned_ptr &operator=(const cloned_ptr &other) {
    if (this != &other) {
      this->std::unique_ptr<T, Deleter>::operator=(other);
      _copier = other._copier;
    }
    return this;
  }

  Copier &get_copier() { return _copier; }
  const Copier &get_copier() const { return _copier; }

private:
  Copier _copier;
};

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_movable<T>()),
    HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
lib::unique_box<T> make_unique_box(Args &&...args) {
  return lib::unique_box<T>(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_copyable<T>()),
    HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
lib::box<T> make_box(Args &&...args) {
  return lib::box<T>(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

template<
    typename T,
    typename... Args,
    HALCHECK_REQUIRE(lib::is_copyable<T>()),
    HALCHECK_REQUIRE(std::is_constructible<T, Args...>())>
lib::cloned_ptr<T> make_cloned_ptr(Args &&...args) {
  return lib::cloned_ptr<T>(lib::in_place_type_t<T>(), std::forward<Args>(args)...);
}

}} // namespace halcheck::lib

#endif
