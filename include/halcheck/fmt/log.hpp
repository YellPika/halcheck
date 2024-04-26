#ifndef HALCHECK_FMT_LOG_HPP
#define HALCHECK_FMT_LOG_HPP

#include <halcheck/fmt/indent.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/variant.hpp>

namespace halcheck { namespace fmt {

struct test_case_start {
  std::uintmax_t iteration;
  friend void operator<<(std::ostream &os, const test_case_start &value) {
    os << "Test Case (" << value.iteration << "): Start";
  }
};

struct test_case_end {
  std::uintmax_t iteration;
  friend void operator<<(std::ostream &os, const test_case_end &value) {
    os << "Test Case (" << value.iteration << "): End";
  }
};

struct test_case_replay {
  std::string filename;
  friend void operator<<(std::ostream &os, const test_case_replay &value) { os << "Replaying " << value.filename; }
};

struct test_case_record {
  std::string filename;
  friend void operator<<(std::ostream &os, const test_case_record &value) { os << "Recording " << value.filename; }
};

struct shrink_start {
  std::exception_ptr exception;
  friend void operator<<(std::ostream &os, const shrink_start &value) {
    {
      fmt::indent indent(os);
      os << "Exception:\n";
      try {
        std::rethrow_exception(value.exception);
      } catch (const std::exception &e) {
        os << e.what();
      } catch (const std::string &e) {
        os << e;
      } catch (const char *e) {
        os << e;
      } catch (...) {
        os << "unknown exception";
      }
    }
    os << "\nShrinking...";
  }
};

struct shrink_end {
  std::uintmax_t iterations;
  std::exception_ptr exception;
  friend void operator<<(std::ostream &os, const shrink_end &value) {
    fmt::indent indent(os);
    os << "Shrinking (" << value.iterations << "): Complete\n";
    try {
      std::rethrow_exception(value.exception);
    } catch (const std::exception &e) {
      os << e.what();
    } catch (const std::string &e) {
      os << e;
    } catch (const char *e) {
      os << e;
    } catch (...) {
      os << "unknown exception";
    }
  }
};

struct shrink_case_start {
  std::uintmax_t iteration;
  friend void operator<<(std::ostream &os, const shrink_case_start &value) {
    os << "Shrinking (" << value.iteration << "): Start";
  }
};

struct shrink_case_end {
  std::uintmax_t iteration;
  std::exception_ptr exception;
  friend void operator<<(std::ostream &os, const shrink_case_end &value) {
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
      os << "Shrinking (" << value.iteration << "): Success\n" << message;
    } else {
      os << "Shrinking (" << value.iteration << "): Failure";
    }
  }
};

struct message : lib::variant<
                     test_case_start,
                     test_case_end,
                     test_case_replay,
                     test_case_record,
                     shrink_start,
                     shrink_end,
                     shrink_case_start,
                     shrink_case_end> {
  struct listener {
    template<typename T>
    void operator()(T value) const {
      os << value;
    }

    std::ostream &os;
  };

  template<typename T>
  message(T value) : variant(std::move(value)) {}

  friend void operator<<(std::ostream &os, const message &value) { lib::visit(listener{os}, variant(value)); }
};

extern const lib::effect<void, const message &> log;

}} // namespace halcheck::fmt

#endif
