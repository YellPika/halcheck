#ifndef HALCHECK_GEN_SAMPLE_HPP
#define HALCHECK_GEN_SAMPLE_HPP

/**
 * @defgroup gen-sample gen/sample
 * @brief Sampling primitives.
 * @ingroup gen
 */

#include <halcheck/gen/label.hpp>
#include <halcheck/lib/atom.hpp>
#include <halcheck/lib/effect.hpp>
#include <halcheck/lib/pp.hpp>

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace halcheck { namespace gen {

/**
 * @brief An effect for generating random data.
 * @ingroup gen-sample
 */
struct sample_effect {
  /**
   * @brief The maximum value that should be returned by handlers of this effect.
   */
  std::uintmax_t max;

  /**
   * @brief By default, `lib::effect::invoke<sample_effect>` throws an exception.
   * @return Does not return.
   */
  std::uintmax_t fallback() const { throw std::runtime_error("sample not handled"); }
};

/**
 * @brief Obtains a random value.
 * @par Signature
 * @code
 *   std::uintmax_t sample(lib::atom id, std::uintmax_t max = std::numeric_limits<std::uintmax_t>::max())
 * @endcode
 * @param id A unique id for the generated value.
 * @param max The maximum value to generate.
 * @return The result of calling `lib::effect::invoke<sample_effect>(max)`, which should be in the range [0, max).
 * @note Calling gen::sample multiple times with the same @p id will typically result in the same value, unless the two
 * calls to gen::sample occur during the lifetime of two calls to gen::label with different `id`s.
 * @ingroup gen-sample
 */
HALCHECK_INLINE_CONSTEXPR struct {
  std::uintmax_t operator()(lib::atom id, std::uintmax_t max = std::numeric_limits<std::uintmax_t>::max()) const {
    auto _ = gen::label(id);
    return lib::effect::invoke<gen::sample_effect>(max);
  }
} sample;

}} // namespace halcheck::gen

#endif
