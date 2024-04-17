#ifndef HALCHECK_FMT_LOG_HPP
#define HALCHECK_FMT_LOG_HPP

#include <halcheck/fmt/indent.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/variant.hpp>

namespace halcheck { namespace fmt {

struct test_case_start {
  std::uintmax_t iteration;
  friend void operator<<(std::ostream &os, const test_case_start &value) {
    os << "Test Case Start (" << value.iteration << ")";
  }
};

struct test_case_end {
  std::uintmax_t iteration;
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
  std::uintmax_t iterations;
  std::exception_ptr exception;
  friend void operator<<(std::ostream &os, const shrink_end &value) {
    std::string message;
    try {
      std::rethrow_exception(value.exception);
    } catch (const std::exception &e) {
      message = e.what();
    } catch (const std::string &e) {
      message = e;
    } catch (const char *e) {
      message = e;
    } catch (...) {
      message = "unknown exception";
    }

    fmt::indent indent(os);
    os << "Shrink Complete (" << value.iterations << "):\n" << message;
  }
};

struct shrink_step {
  std::uintmax_t iteration;
  std::exception_ptr exception;
  friend void operator<<(std::ostream &os, const shrink_step &value) {
    if (value.exception) {
      std::string message;
      try {
        std::rethrow_exception(value.exception);
      } catch (const std::exception &e) {
        message = e.what();
      } catch (const std::string &e) {
        message = e;
      } catch (const char *e) {
        message = e;
      } catch (...) {
        message = "unknown exception";
      }

      fmt::indent indent(os);
      os << "Shrink Success (" << value.iteration << "):\n" << message;
    } else {
      os << "Shrink Failure (" << value.iteration << ")";
    }
  }
};

struct message : lib::variant<
                     test_case_start,
                     test_case_end,
                     test_case_replay,
                     test_case_capture,
                     shrink_start,
                     shrink_end,
                     shrink_step> {
  struct listener {
    template<typename T>
    void operator()(T value) const {
      os << value;
    }

    std::ostream &os;
  };

  template<typename T>
  message(T value) : variant(std::move(value)) {}

  friend void operator<<(std::ostream &os, const message &value) { lib::visit(listener{os}, value); }
};

extern const lib::effect<void, const message &> log;

}} // namespace halcheck::fmt

#endif
