#ifndef HALCHECK_FMT_SHOWABLE_HPP
#define HALCHECK_FMT_SHOWABLE_HPP

#include <halcheck/fmt/show.hpp>
#include <halcheck/lib/effect.hpp>

#include <memory>
#include <ostream>

namespace halcheck { namespace fmt {

class showable {
public:
  template<typename T>
  showable(T value) : _impl(new derived<T>(std::move(value))) {}

  bool operator==(const showable &other) const { return _impl->equals(*other._impl); }

private:
  friend std::ostream &operator<<(std::ostream &os, const showable &value) { return value._impl->show(os); }

  struct base {
    virtual std::ostream &show(std::ostream &os) const = 0;
    virtual bool equals(const base &other) const = 0;
    virtual const void *data() const = 0;
    virtual std::size_t id() const = 0;
  };

  template<typename T>
  class derived final : public base {
  public:
    explicit derived(T value) : _value(std::move(value)) {}

    std::ostream &show(std::ostream &os) const override { return os << fmt::show(_value); }

    bool equals(const base &other) const override {
      return _id == other.id() && _value == *reinterpret_cast<const T *>(other.data());
    }

    const void *data() const override { return reinterpret_cast<const void *>(&_value); }

    std::size_t id() const override { return _id; }

  private:
    static std::size_t _id;

    T _value;
  };

  std::shared_ptr<base> _impl;
};

}} // namespace halcheck::fmt

#endif
