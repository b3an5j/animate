#include "pixel_helper.h"

color_t* pixel_get_addr(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return &grid[x + (y * width)];
}

color_t pixel_get_color(color_t* grid, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    return *pixel_get_addr(grid, x, y, height, width);
}

void pixel_set_color(color_t* grid, color_t value, ssize_t x, ssize_t y,
    size_t height, size_t width)
{
    *pixel_get_addr(grid, x, y, height, width) = value;
}