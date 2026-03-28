#include "pixel_helper.h"

inline color_t* pixel_get_addr(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return &grid[x + (y * width)];
}

inline color_t pixel_get_color(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return *pixel_get_addr(grid, x, y, height, width);
}

inline void pixel_set_color(color_t* grid, color_t value, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    *pixel_get_addr(grid, x, y, height, width) = value;
}

inline ssize_t pixel_get_positive_coord(ssize_t coord, size_t length)
{
    if (!length) { return 0; }
    ssize_t res = coord % (ssize_t)length;
    return (res < 0) ? (res + (ssize_t)length) : res;
}