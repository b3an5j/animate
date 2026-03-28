#ifndef PIXEL_HELPER_H
#define PIXEL_HELPER_H

#include "animate.h"
#include <sys/types.h>

typedef union {
    color_t raw;
    struct {
        uint8_t A;
        uint8_t R;
        uint8_t G;
        uint8_t B;
    } colors;
} Channel;

color_t* pixel_get_addr(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width);
color_t pixel_get_color(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width);
void pixel_set_color(color_t* grid, color_t value, ssize_t x, ssize_t y,
    size_t height, size_t width);

static inline ssize_t pixel_get_positive_coord(ssize_t coord, size_t length)
{
    if (!length) { return 0; }
    ssize_t res = coord % (ssize_t)length;
    return (res < 0) ? (res + (ssize_t)length) : res;
}

#endif