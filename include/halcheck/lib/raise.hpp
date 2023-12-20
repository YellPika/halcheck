#ifndef HALCHECK_LIB_RAISE_HPP
#define HALCHECK_LIB_RAISE_HPP

#include <stdexcept>

namespace halcheck { namespace lib {

struct raise {
  raise() = delete;

  template<typename T>
  raise(const T &value) {
    throw value;
  }

  template<typename T>
  [[noreturn]] operator T &() const {
    throw std::runtime_error("halcheck::lib::raise: impossible");
  }
};

}} // namespace halcheck::lib

#endif
