#include "halcheck/clang/fuzz.hpp"

#ifdef __clang__
#include <halcheck/eff/api.hpp>
#include <halcheck/gen/discard.hpp>
#include <halcheck/gen/label.hpp>
#include <halcheck/gen/sample.hpp>
#include <halcheck/gen/size.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/optional.hpp>
#include <halcheck/lib/scope.hpp>
#include <halcheck/lib/tree.hpp>
#include <halcheck/lib/variant.hpp>
#include <halcheck/test/strategy.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace halcheck;

namespace {

struct handler : eff::handler<handler, gen::sample_effect, gen::label_effect, gen::size_effect> {
  handler(std::uintmax_t max_size, std::uintmax_t max_length, const uint8_t *data, size_t len)
      : data(data), len(len), size(len * max_size / max_length) {}

  std::uintmax_t operator()(gen::sample_effect args) override {
    auto clamp = [&](std::uintmax_t output) { return std::min(output, args.max); };
    if (auto output = **current)
      return clamp(*output);
    else if (args.max <= std::numeric_limits<std::uint8_t>::max() && len >= sizeof(std::uint8_t))
      return clamp(read<std::uint8_t>());
    else if (args.max <= std::numeric_limits<std::uint16_t>::max() && len >= sizeof(std::uint16_t))
      return clamp(read<std::uint16_t>());
    else if (args.max <= std::numeric_limits<std::uint32_t>::max() && len >= sizeof(std::uint32_t))
      return clamp(read<std::uint32_t>());
    else if (args.max <= std::numeric_limits<std::uint64_t>::max() && len >= sizeof(std::uint64_t))
      return clamp(read<std::uint64_t>());
    else
      return 0;
  }

  lib::destructable operator()(gen::label_effect args) override {
    auto prev = current;
    current = &(*current)[args.value];
    auto self = this;
    return lib::finally([=] { self->current = prev; });
  }

  std::uintmax_t operator()(gen::size_effect) override { return size; }

  template<typename T>
  T read() {
    T output;
    std::memcpy(&output, data, sizeof(T));
    **current = output;
    len -= sizeof(T);
    data += sizeof(T);
    return output;
  }

  const std::uint8_t *data;
  std::size_t len;
  std::uintmax_t size;
  lib::tree<lib::atom, lib::optional<std::uintmax_t>> state;
  lib::tree<lib::atom, lib::optional<std::uintmax_t>> *current = &state;
};
} // namespace

static std::uintmax_t max_size;
static std::uintmax_t max_length;
static std::function<void()> func;

static int entry(const std::uint8_t *data, std::size_t len) {
  ::handler handler(max_size, max_length, data, len);

  try {
    eff::handle(func, handler);
    return 0;
  } catch (const gen::discard &) {
    return -1;
  }
};

static std::vector<char> to_vector(const std::string &str) {
  std::vector<char> output(str.begin(), str.end());
  output.push_back('\0');
  return output;
}

extern "C" int LLVMFuzzerRunDriver(int *, char ***, int (*)(const std::uint8_t *, std::size_t));

test::strategy clang::fuzz(
    std::uintmax_t max_size, // NOLINT
    std::uintmax_t max_length,
    const std::vector<std::string> &args) {
  return [=](std::function<void()> func) {
    std::vector<std::vector<char>> copy;
    std::vector<char *> pointers;
    copy.push_back(to_vector("./fuzzer"));
    pointers.push_back(copy.back().data());
    for (auto &&arg : args) {
      copy.push_back(to_vector(arg));
      pointers.push_back(copy.back().data());
    }
    copy.push_back(to_vector("-max_len=" + std::to_string(max_length)));
    pointers.push_back(copy.back().data());

    int argc = int(copy.size());
    char **argv = pointers.data();

    ::func = std::move(func);
    ::max_size = max_size;
    ::max_length = max_length;
    try {
      LLVMFuzzerRunDriver(&argc, &argv, entry);
    } catch (const gen::succeed &) {
    }
  };
}

#endif
