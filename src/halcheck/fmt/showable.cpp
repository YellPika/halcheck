#include "halcheck/fmt/showable.hpp"

#include <atomic>

using namespace halcheck;

static std::atomic_size_t counter;

template<typename T>
std::size_t fmt::showable::derived<T>::_id = counter++;
