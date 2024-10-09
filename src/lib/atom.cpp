#include "halcheck/lib/atom.hpp"

#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

using namespace halcheck;

lib::symbol::symbol(const std::string &value) {
  static std::mutex mutex;
  static std::unordered_map<std::string, std::size_t> hashes;

  std::lock_guard<std::mutex> lock(mutex);

  auto it = hashes.find(value);
  if (it == hashes.end())
    it = hashes.emplace(value, std::hash<std::string>()(value)).first;

  _data = &*it;
}

lib::symbol::symbol(const char *value) : symbol(std::string(value)) {}
