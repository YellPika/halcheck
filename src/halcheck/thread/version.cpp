#include "halcheck/thread/version.hpp"

#include <halcheck/lib/scope.hpp>

#include <atomic>

using namespace halcheck;

thread::version &thread::version::operator|=(const thread::version &other) {
  if (this != &other) {
    for (auto &&pair : other.counters) {
      auto &counter = counters[pair.first];
      counter = std::max(counter, pair.second);
    }
  }

  return *this;
}

thread::version thread::version::operator|(thread::version other) const {
  other |= *this;
  return other;
}

bool thread::version::operator<(const thread::version &other) const {
  bool result = false;
  for (auto &&pair : counters) {
    auto it = other.counters.find(pair.first);
    if (it == other.counters.end() || pair.second > it->second)
      return false;

    if (pair.second < it->second)
      result = true;
  }
  return result;
}

bool thread::version::operator==(const thread::version &other) const { return counters == other.counters; }

static std::size_t id() {
  static std::atomic_size_t next{0};
  static thread_local std::size_t id = next++;
  return id;
}

static thread_local thread::version _current;

thread::version thread::version::next() {
  ++_current.counters[id()];
  return _current;
}

void thread::version::join(const thread::version &other) { _current |= other; }

namespace halcheck { namespace thread {

std::ostream &operator<<(std::ostream &os, const version &value) {
  os << "{";
  if (!value.counters.empty()) {
    auto i = value.counters.begin();
    os << i->first << ": " << i->second;
    for (++i; i != value.counters.end(); ++i)
      os << ", " << i->first << ": " << i->second;
  }
  os << "}";
  return os;
}

}} // namespace halcheck::thread
