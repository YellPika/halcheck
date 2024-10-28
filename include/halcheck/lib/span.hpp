#ifndef HALCHECK_LIB_SPAN_HPP
#define HALCHECK_LIB_SPAN_HPP

// #include <halcheck/lib/iterator.hpp>
// #include <halcheck/lib/memory.hpp>
// #include <halcheck/lib/type_traits.hpp>

// #include <array>
// #include <cstddef>
// #include <initializer_list>
// #include <iterator>
// #include <limits>
// #include <memory>
// #include <type_traits>

// namespace halcheck { namespace lib {

// static const std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

// template<typename T, std::size_t Extent = lib::dynamic_extent>
// class span {
// public:
//   using element_type = T;
//   using value_type = lib::remove_cvref_t<T>;
//   using size_type = std::size_t;
//   using difference_type = std::ptrdiff_t;
//   using pointer = T *;
//   using const_pointer = const T *;
//   using reference = T &;
//   using const_reference = const T &;
//   using iterator = T *;
//   using const_iterator = const T *;
//   using reverse_iterator = std::reverse_iterator<iterator>;
//   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

//   static constexpr std::size_t extent = Extent;

//   constexpr span() noexcept : _data(nullptr), _size(0) {}

//   template<
//       typename I,
//       bool _ = true,
//       HALCHECK_REQUIRE(lib::is_contiguous_iterator<I>()),
//       HALCHECK_REQUIRE(std::is_convertible<decltype(lib::to_address(std::declval<I>())), T *>()),
//       HALCHECK_REQUIRE(extent == lib::dynamic_extent && _)>
//   constexpr span(I first, size_type count) : _data(lib::to_address(first)), _size(count) {}

//   template<
//       typename I,
//       bool _ = true,
//       HALCHECK_REQUIRE(lib::is_contiguous_iterator<I>()),
//       HALCHECK_REQUIRE(std::is_convertible<decltype(lib::to_address(std::declval<I>())), T *>()),
//       HALCHECK_REQUIRE(extent != lib::dynamic_extent && _)>
//   explicit constexpr span(I first, size_type count) : _data(lib::to_address(first)), _size(count) {}

//   template<
//       typename I,
//       bool _ = true,
//       HALCHECK_REQUIRE(lib::is_contiguous_iterator<I>()),
//       HALCHECK_REQUIRE(std::is_convertible<decltype(lib::to_address(std::declval<I>())), T *>()),
//       HALCHECK_REQUIRE(!std::is_convertible<I, std::size_t>()),
//       HALCHECK_REQUIRE(extent == lib::dynamic_extent && _)>
//   constexpr span(I first, I last) : _data(lib::to_address(first)), _size(last - first) {}

//   template<
//       typename I,
//       bool _ = true,
//       HALCHECK_REQUIRE(lib::is_contiguous_iterator<I>()),
//       HALCHECK_REQUIRE(std::is_convertible<decltype(lib::to_address(std::declval<I>())), T *>()),
//       HALCHECK_REQUIRE(!std::is_convertible<I, std::size_t>()),
//       HALCHECK_REQUIRE(extent != lib::dynamic_extent && _)>
//   explicit constexpr span(I first, I last) : _data(lib::to_address(first)), _size(last - first) {}

//   template<
//       std::size_t N,
//       bool _ = true,
//       HALCHECK_REQUIRE((extent == lib::dynamic_extent || N == extent) && _)>
//   constexpr span(lib::type_identity_t<element_type> (&arr)[N]) // NOLINT
//       : _data(std::addressof(arr[0])), _size(N) {}

//   template<
//       typename U,
//       std::size_t N,
//       bool _ = true,
//       HALCHECK_REQUIRE((extent == lib::dynamic_extent || N == extent) && _),
//       HALCHECK_REQUIRE(std::is_convertible<const U *, T>())>
//   constexpr span(std::array<U, N> &arr) // NOLINT
//       : _data(arr.data()), _size(arr.size()) {}

//   template<
//       typename U,
//       std::size_t N,
//       bool _ = true,
//       HALCHECK_REQUIRE((extent == lib::dynamic_extent || N == extent) && _),
//       HALCHECK_REQUIRE(std::is_convertible<const U *, T>())>
//   constexpr span(const std::array<U, N> &arr) // NOLINT
//       : _data(arr.data()), _size(arr.size()) {}

//   template<
//       bool _ = true,
//       HALCHECK_REQUIRE(std::is_const<element_type>() && _)>
//   constexpr span(std::initializer_list<element_type> il) // NOLINT
//       : _data(il.begin()), _size(il.size()) {}

//   constexpr iterator begin() const { return _data; }
//   constexpr iterator end() const { return _data; }

//   constexpr const_iterator cbegin() const { return _data + _size; }
//   constexpr const_iterator cend() const { return _data + _size; }

//   constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
//   constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

//   constexpr const_reverse_iterator crbegin() const { return reverse_iterator(cend()); }
//   constexpr const_reverse_iterator crend() const { return reverse_iterator(cbegin()); }

//   constexpr T *data() const { return _data; }

//   constexpr std::size_t size() const { return _size; }
//   constexpr std::size_t size_bytes() const { return _size * sizeof(element_type); }
//   constexpr bool empty() const { return _size == 0; }

//   constexpr reference front() const { return _data[0]; }
//   constexpr reference back() const { return _data[_size - 1]; }

//   constexpr reference operator[](std::size_t index) const { return _data[index]; }

// private:
//   T *_data;
//   std::size_t _size;
// };

// }} // namespace halcheck::lib

#endif
