#ifndef HALCHECK_FMT_LOG_HPP
#define HALCHECK_FMT_LOG_HPP

#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/variant.hpp>

namespace halcheck { namespace fmt {

struct test_case_start {
  int iteration;
  friend void operator<<(std::ostream &os, const test_case_start &value) {
    os << "Test Case Start (" << value.iteration << ")";
  }
};

struct test_case_end {
  int iteration;
  friend void operator<<(std::ostream &os, const test_case_end &value) {
    os << "Test Case End (" << value.iteration << ")";
  }
};

struct test_case_replay {
  std::string filename;
  friend void operator<<(std::ostream &os, const test_case_replay &value) { os << "Replaying " << value.filename; }
};

struct test_case_capture {
  std::string filename;
  friend void operator<<(std::ostream &os, const test_case_capture &value) { os << "Capturing " << value.filename; }
};

struct shrink_start {
  friend void operator<<(std::ostream &os, const shrink_start &) { os << "Shrinking..."; }
};

struct shrink_end {
  int iterations;
  friend void operator<<(std::ostream &os, const shrink_end &value) {
    os << "Completed Shrinking (" << value.iterations << ")";
  }
};

struct shrink_success {
  int iteration;
  friend void operator<<(std::ostream &os, const shrink_success &value) {
    os << "Shrinking (" << value.iteration << ")";
  }
};

struct message : lib::variant<
                     test_case_start,
                     test_case_end,
                     test_case_replay,
                     test_case_capture,
                     shrink_start,
                     shrink_end,
                     shrink_success> {
  struct listener {
    template<typename T>
    void operator()(T value) const {
      os << value;
    }

    std::ostream &os;
  };

  using variant::variant;

  friend void operator<<(std::ostream &os, const message &value) { lib::visit(listener{os}, value); }
};

extern const lib::effect<void, const message &> log;

}} // namespace halcheck::fmt

#endif
