#ifndef MISC_HELPER_H
#define MISC_HELPER_H

#include "animate.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

// https://edstem.org/au/courses/31567/discussion/3160179
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) do {\
    fprintf(stderr, "DEBUG: " fmt, __VA_ARGS__);} while(0)
#else
#define DBG_PRINT(fmt, ...) do {} while(0)
#endif

bool validate_dimension(size_t height, size_t width);

#endif