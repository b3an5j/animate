/**
 * @file misc_helper.h
 * @brief Helpers
*/
#ifndef MISC_HELPER_H
#define MISC_HELPER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @brief Enum for debug printing
 */
typedef enum {
    ERR_MALLOC,         /**< Malloc error */
    ERR_MALLOC_WSIZE,   /**< Malloc error, with size */
    ERR_IO,             /**< I/O error */
    ERR_BM_HEADER,      /**< Bitmap header error */
    ERR_BM_V5,          /**< Bitmap V5 header error */
    FREED,              /**< Memory allocation freed */
    INVALID_ARG,        /**< Invalid argument */
    INVALID_DIM,        /**< Invalid dimension */
    TOOBIG,             /**< Size is too big */
    FAIL,               /**< Task is unsuccessful */
    SUCCESS,            /**< Task is successful */
    CUSTOM              /**< Custom debug message */
} ErrorType;

/**
 * @brief String to be printed in debug printing
 */
extern const char* const dbg_strings[];

// https://edstem.org/au/courses/31567/discussion/3160179
/**
 * @def DBG_PRINT(code, ...)
 * @brief Prints debug info
 */
#ifdef DEBUG
#include <stdarg.h>
 /** @cond INTERNAL */
void dbg_print_helper(ErrorType code, ...);
/** @endcond */
#define DBG_PRINT(code, ...) dbg_print_helper(code, __VA_ARGS__)
#else
#define DBG_PRINT(code, ...) do {} while(0)
#endif

 /**
  * @brief Validates the size of area. Prevent crazy allocations.
  *
  * @param height Height in pixels
  * @param width Width in pixels
  * @return true if valid
  * @return false if invalid
  */
bool validate_dimension(size_t height, size_t width);

#endif /* MISC_HELPER_H */