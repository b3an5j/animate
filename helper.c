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

bool validate_dimension(size_t height, size_t width)
{
    // a bit dangerous since height and width are size_t, safety check?
    if (!height || !width || height > UINT32_MAX || width > UINT32_MAX) {
        DBG_PRINT("Invalid dimension, H = %zu W = %zu\n", height, width);
        return false;
    }
    return true;
}