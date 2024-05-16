#include "halcheck/lib/thread.hpp"

using namespace halcheck;

void lib::version::bump(std::size_t i) { ++counters[i]; }

lib::version &lib::version::operator|=(const lib::version &other) {
  for (auto &&pair : other.counters) {
    auto &counter = counters[pair.first];
    counter = std::max(counter, pair.second);
  }
  return *this;
}

bool lib::version::operator<(const lib::version &other) const {
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

bool lib::version::operator==(const lib::version &other) const { return counters == other.counters; }

namespace halcheck { namespace lib {
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
}} // namespace halcheck::lib
