#ifndef HALCHECK_LIB_PP_HPP
#define HALCHECK_LIB_PP_HPP

/**
 * @defgroup lib-pp lib/pp
 * @brief Pre-processing utilities.
 * @ingroup lib
 */

/**
 * @brief Literally nothing.
 * @ingroup lib-pp
 */
#define HALCHECK_EMPTY

/**
 * @brief Delays the expansion of a token sequence.
 * @ingroup lib-pp
 */
#define HALCHECK_EXPAND(x) x

#define HALCHECK_CAT_HELPER(x, y) x##y

/**
 * @brief Concatenates two token sequences.
 * @ingroup lib-pp
 */
#define HALCHECK_CAT(x, y) HALCHECK_CAT_HELPER(x, y)

#define HALCHECK_1ST_HELPER(x, ...) x

/**
 * @brief Retrieves the first element of a list of token sequences.
 * @ingroup lib-pp
 */
#define HALCHECK_1ST(...) HALCHECK_EXPAND(HALCHECK_1ST_HELPER(__VA_ARGS__, HALCHECK_EMPTY))

#define HALCHECK_2ND_HELPER(x, y, ...) y

/**
 * @brief Retrieves the second element of a list of token sequences.
 * @ingroup lib-pp
 */
#define HALCHECK_2ND(...) HALCHECK_EXPAND(HALCHECK_2ND_HELPER(__VA_ARGS__, HALCHECK_EMPTY, HALCHECK_EMPTY))

#define HALCHECK_3RD_HELPER(x, y, z, ...) z

/**
 * @brief Retrieves the third element of a list of token sequences.
 * @ingroup lib-pp
 */
#define HALCHECK_3RD(...)                                                                                              \
  HALCHECK_EXPAND(HALCHECK_3RD_HELPER(__VA_ARGS__, HALCHECK_EMPTY, HALCHECK_EMPTY, HALCHECK_EMPTY))

#if defined(HALCHECK_DOXYGEN) || __cplusplus >= 201703L
/**
 * @brief A backwards-compatible substitute for the [[nodiscard]] attribute.
 * @ingroup lib-pp
 */
#define HALCHECK_NODISCARD [[nodiscard]]
#else
#define HALCHECK_NODISCARD
#endif

#endif
