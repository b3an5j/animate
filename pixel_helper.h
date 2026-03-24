#ifndef PIXEL_HELPER_H
#define PIXEL_HELPER_H

#include "animate.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

bool validate_dimension(size_t height, size_t width);

color_t get_pixel(color_t* grid, uint32_t x, uint32_t y, uint32_t width);
color_t* get_pixel_addr(color_t* grid, uint32_t x, uint32_t y, uint32_t width);
void set_pixel(color_t* grid, color_t value, uint32_t x, uint32_t y, uint32_t width);

#endif