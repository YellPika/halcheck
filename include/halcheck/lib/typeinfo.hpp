#ifndef HALCHECK_LIB_RTTI_HPP
#define HALCHECK_LIB_RTTI_HPP

#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define HALCHECK_RTTI
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define HALCHECK_RTTI
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define HALCHECK_RTTI
#endif
#endif

#ifdef HALCHECK_RTTI
#include <string>
#include <typeinfo>
#endif

namespace halcheck { namespace lib {

#ifdef HALCHECK_RTTI

/// @brief Gets the demangled name associated with a std::type_info.
/// @param info The std::type_info to extract a name from.
/// @return The std::string representation of info's name.
std::string nameof(const std::type_info &info);

/// @brief Gets the demangled name associated with a type.
/// @tparam T The type to extract a name from.
/// @return The std::string representation of the T's name.
template<typename T>
std::string nameof() {
  return nameof(typeid(T));
}
#endif

}} // namespace halcheck::lib

#endif
