#ifndef MISC_HELPER_H
#define MISC_HELPER_H

#include "animate.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

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
    CUSTOM
} ErrorType;

extern const char* dbg_strings[];

// https://edstem.org/au/courses/31567/discussion/3160179
#ifdef DEBUG
#define DBG_PRINT(code, ...) do {\
    fprintf(stderr, "DEBUG: ");\
    if ((code) == CUSTOM) {\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, "\n");\
    }\
    else {\
        fprintf(stderr, dbg_strings[code], __VA_ARGS__);\
    }\
} while (0)
#else
#define DBG_PRINT(fmt, ...) do {} while(0)
#endif

bool validate_dimension(size_t height, size_t width);

#endif