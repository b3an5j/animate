#include "pixel_helper.h"

color_t get_pixel(color_t* grid, uint32_t x, uint32_t y, uint32_t width)
{
    return grid[x + (y * width)];
}

color_t* get_pixel_addr(color_t* grid, uint32_t x, uint32_t y, uint32_t width)
{
    return &grid[x + (y * width)];
}

void set_pixel_(color_t* grid, color_t value, uint32_t x, uint32_t y, uint32_t width)
{
    grid[x + (y * width)] = value;
}