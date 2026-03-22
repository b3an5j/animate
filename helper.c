#include "helper.h"

/* Assumed ARGB32 Little Endian */
union channel {
    color_t raw;
    struct {
        uint8_t B;
        uint8_t G;
        uint8_t R;
        uint8_t A;
    };
};

color_t get_pixel(color_t* grid, int x, int y, int width)
{
    return grid[x + (y * width)];
}

void set_pixel(color_t* grid, color_t value, int x, int y, int width)
{
    grid[x + (y * width)] = value;
}