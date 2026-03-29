#ifndef PIXEL_HELPER_H
#define PIXEL_HELPER_H

#include "animate.h"
#include <sys/types.h>

/* ASSUME LITTLE ENDIAN ARGB */
typedef union {
    color_t raw;
    struct {
        uint8_t B;
        uint8_t G;
        uint8_t R;
        uint8_t A;
    } colors;
} Channel;

static inline color_t* pixel_get_addr(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return &grid[x + (y * width)];
}

static inline color_t pixel_get_color(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return *pixel_get_addr(grid, x, y, height, width);
}

static inline void pixel_set_color(color_t* grid, color_t value, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    /* set A channel to xFF if not 0 */
    value |= 0xFF << 24;
    *pixel_get_addr(grid, x, y, height, width) = value;
}

static inline ssize_t pixel_get_positive_coord(ssize_t coord, size_t length)
{
    if (!length) { return 0; }
    ssize_t res = coord % (ssize_t)length;
    return (res < 0) ? (res + (ssize_t)length) : res;
}

#endif