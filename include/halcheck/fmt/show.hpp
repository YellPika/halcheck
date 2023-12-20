#ifndef HALCHECK_FMT_SHOW_HPP
#define HALCHECK_FMT_SHOW_HPP

#include <halcheck/fmt/indent.hpp>
#include <halcheck/fmt/infer.hpp>
#include <halcheck/fmt/type.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/ranges.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/tuple.hpp>
#include <halcheck/lib/type_traits.hpp>
#include <halcheck/lib/utility.hpp>
#include <halcheck/lib/variant.hpp>

#include <cstddef>
#include <sstream>
#include <type_traits>
#include <utility>

namespace halcheck { namespace fmt {

/// @brief A proxy type used for printing values. For custom printing, provide
///        an overload of operator<<(std::ostream &, fmt::tag<T>) for your
///        type. Printed values should ideally be compilable code that evaluates
///        to the original value.
/// @tparam T The type of value to print.
template<typename T>
struct tag {
  const T &value;
};

/// @brief Constructs a tag instance which can be printed to a std::ostream.
/// @tparam T The type of value to print.
/// @param value The value to print.
/// @return A value of type fmt::tag<T> containing value.
template<typename T>
constexpr tag<T> show(const T &value) {
  return {value};
}

template<typename T>
constexpr tag<std::initializer_list<T>> show(const std::initializer_list<T> &value) {
  return {value};
}

template<typename T>
std::ostream &operator<<(std::ostream &os, tag<T> value);

/// @brief Converts a value to a std::string using
///        operator<<(std::ostream &, fmt:tag<T>).
/// @tparam T The type of value to stringify.
/// @param value The value to stringify.
/// @return A std::string containing the string representation of value.
template<typename T>
std::string to_string(const T &value) {
  std::ostringstream os;
  os << fmt::show<T>(value);
  return os.str();
}

namespace detail {
void escape(std::ostream &os, char);

enum priority_t {
  PRIORITY_POINTER,
  PRIORITY_ARITHMETIC,
  PRIORITY_STREAMABLE,
  PRIORITY_TUPLE,
  PRIORITY_RANGE,
  PRIORITY_DEFAULT
};

template<typename T>
constexpr priority_t priority() {
  return std::is_pointer<T>()                                                              ? PRIORITY_POINTER
         : std::is_arithmetic<T>() || (std::is_enum<T>() && !lib::is_streamable_enum<T>()) ? PRIORITY_ARITHMETIC
         : lib::is_streamable<T>() && !std::is_array<T>()                                  ? PRIORITY_STREAMABLE
         : lib::is_tuple_like<T>()                                                         ? PRIORITY_TUPLE
         : lib::is_range<T>()                                                              ? PRIORITY_RANGE
                                                                                           : PRIORITY_DEFAULT;
}

template<priority_t>
struct print;

template<>
struct print<PRIORITY_POINTER> {

  template<typename T, HALCHECK_REQUIRE(std::is_void<T>())>
  print(std::ostream &os, T *value) {
    if (!value)
      os << "nullptr";
    else
      os << value;
  }

  template<typename T, HALCHECK_REQUIRE(!std::is_void<T>())>
  print(std::ostream &os, T *value) {
    if (!value)
      os << "nullptr";
    else if (std::is_copy_constructible<T>() || std::is_move_constructible<T>())
      os << "new " << fmt::type<T>() << "(" << fmt::show(*value) << ")";
    else
      os << fmt::show(*value) << " (" << value << ")";
  }
};

template<>
struct print<PRIORITY_ARITHMETIC> {
  print(std::ostream &os, bool value) { os << (value ? "true" : "false"); }

  print(std::ostream &os, char value) {
    os << '\'';
    fmt::detail::escape(os, value);
    os << '\'';
  }

  print(std::ostream &os, unsigned char value) {
    os << '\'';
    fmt::detail::escape(os, value);
    os << '\'';
  }

  print(std::ostream &os, signed char value) {
    os << '\'';
    fmt::detail::escape(os, value);
    os << '\'';
  }

  print(std::ostream &os, int value) { os << value; }

  print(std::ostream &os, unsigned int value) {
    os << value;
    if (!fmt::infer(os))
      os << "U";
  }

  print(std::ostream &os, long value) {
    os << value;
    if (!fmt::infer(os))
      os << "L";
  }

  print(std::ostream &os, unsigned long value) {
    os << value;
    if (!fmt::infer(os))
      os << "UL";
  }

  print(std::ostream &os, long long value) {
    os << value;
    if (!fmt::infer(os))
      os << "LL";
  }

  print(std::ostream &os, unsigned long long value) {
    os << value;
    if (!fmt::infer(os))
      os << "ULL";
  }

  print(std::ostream &os, float value) {
    print(os, double(value));
    if (!fmt::infer(os))
      os << "f";
  }

  print(std::ostream &os, double value) {
    auto output = std::to_string(value);
    while (output.back() == '0')
      output.pop_back();
    if (output.back() == '.')
      output.push_back('0');
    os << output;
  }

  template<typename T>
  print(std::ostream &os, T value) {
    if (!fmt::infer(os))
      os << "(" << fmt::type<T>() << ")";

    os << value;
  }
};

template<>
struct print<PRIORITY_STREAMABLE> {
  template<typename Traits, typename Alloc>
  print(std::ostream &os, const std::basic_string<char, Traits, Alloc> &value) {
    if (!fmt::infer(os))
      os << fmt::type<std::basic_string<char, Traits, Alloc>>() << "(";

    os << '"';
    for (auto &ch : value)
      escape(os, ch);
    os << '"';

    if (!fmt::infer(os))
      os << ")";
  }

  template<typename T>
  print(std::ostream &os, const T &value) {
    os << value;
  }
};

template<>
struct print<PRIORITY_TUPLE> {
  template<typename T>
  print(std::ostream &os, const T &, lib::index_sequence<>) {
    if (!fmt::infer(os))
      os << fmt::type<T>();

    os << "{}";
  }

  template<typename T>
  print(std::ostream &os, const T &value, lib::index_sequence<0>) {
    if (!fmt::infer(os))
      os << fmt::type<T>();

    os << "{" << fmt::show(value) << "}";
  }

  template<typename T, std::size_t... I>
  print(std::ostream &os, const T &value, lib::index_sequence<0, I...>) {
    auto infer = fmt::infer(os, !fmt::infer(os));
    auto reset = lib::finally([&] { fmt::infer(os, infer); });

    fmt::indent indent(os, infer ? 1 : 4);
    if (!infer)
      os << fmt::type<T>();

    os << (infer ? "{" : "{\n") << fmt::show(std::get<0>(value));
    lib::ignore = {(os << ", \n" << fmt::show(std::get<I>(value)), 0)...};
    os << "}";
  }

  template<typename T>
  print(std::ostream &os, const T &value) {
    print(os, value, lib::make_index_sequence<std::tuple_size<T>::value>());
  }
};

template<>
struct print<PRIORITY_RANGE> {
  template<std::size_t N>
  print(std::ostream &os, const char (&value)[N]) {
    auto old = fmt::infer(os, true);
    auto reset = lib::finally([&] { fmt::infer(os, old); });
    os << fmt::show(std::string(value, N - 1));
  }

  template<typename T>
  print(std::ostream &os, const T &value) {
    auto infer = fmt::infer(os, !fmt::infer(os));
    auto reset = lib::finally([&] { fmt::infer(os, infer); });

    fmt::indent indent(os, infer ? 1 : 4);
    if (!infer)
      os << fmt::type<T>();

    auto begin = lib::begin(value);
    auto end = lib::end(value);
    switch (std::distance(begin, end)) {
    case 0:
      os << "{}";
      break;
    case 1:
      os << "{" << fmt::show(*begin) << "}";
      break;
    default:
      os << (infer ? "{" : "{\n") << fmt::show(*begin++);
      while (begin != end)
        os << ", \n" << fmt::show(*begin++);
      os << "}";
      break;
    }
  }
};

template<>
struct print<PRIORITY_DEFAULT> {
  struct variant_visitor {
    template<typename T>
    void operator()(T value) const {
      os << fmt::show(value);
    }
    std::ostream &os;
  };

  print(std::ostream &os, const std::nullptr_t &) { os << "nullptr"; }

  print(std::ostream &os, const lib::nullopt_t &) { os << "halcheck::lib::nullopt"; }

  template<typename... Args>
  print(std::ostream &os, const lib::variant<Args...> &value) {
    fmt::indent indent(os);
    os << fmt::type<lib::variant<Args...>>() << "(\n";
    os << "halcheck::lib::in_place_index_t<" << value.index() << ">(), \n";
    auto old = fmt::infer(os, true);
    auto reset = lib::finally([&] { fmt::infer(os, old); });
    lib::visit(variant_visitor{os}, value);
    os << ")";
  }

  template<typename T>
  print(std::ostream &os, const lib::optional<T> &value) {
    if (fmt::infer(os)) {
      if (value)
        os << fmt::show(*value);
      else
        os << fmt::show(lib::nullopt);
    } else {
      if (value)
        os << fmt::type<lib::optional<T>>() << "(" << fmt::show(*value) << ")";
      else
        os << fmt::type<lib::optional<T>>() << "()";
    }
  }

  template<typename T>
  print(std::ostream &os, const T &) {
    if (!fmt::infer(os))
      os << fmt::type<T>();

    os << "(...)";
  }
};

} // namespace detail

template<typename T>
std::ostream &operator<<(std::ostream &os, tag<T> value) {
  detail::print<detail::priority<T>()>(os, value.value);
  return os;
}

}} // namespace halcheck::fmt

#endif
