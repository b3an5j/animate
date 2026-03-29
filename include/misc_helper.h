#ifndef MISC_HELPER_H
#define MISC_HELPER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef enum {
    ERR_MALLOC,
    ERR_MALLOC_WSIZE,
    ERR_IO,
    ERR_BM_HEADER,
    ERR_BM_V5,
    FREED,
    INVALID_ARG,
    INVALID_DIM,
    TOOBIG,
    FAIL,
    SUCCESS,
    CUSTOM
} ErrorType;

extern const char* const dbg_strings[];

// https://edstem.org/au/courses/31567/discussion/3160179
#ifdef DEBUG
#include <stdarg.h>
void dbg_print_helper(ErrorType code, ...);
#define DBG_PRINT(code, ...) dbg_print_helper(code, __VA_ARGS__)
#else
#define DBG_PRINT(code, ...) do {} while(0)
#endif

bool validate_dimension(size_t height, size_t width);

#endif