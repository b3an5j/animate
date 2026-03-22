#ifndef ANIM_HELPER_H
#define ANIM_HELPER_H

#include "animate.h"
#include <stdio.h>

union channel;

// https://edstem.org/au/courses/31567/discussion/3160179
#ifdef DEBUG
#define DBG_PRINT(fmt, ...) do {\
    fprintf(stderr, "DEBUG: " fmt, __VA_ARGS__);} while(0)
#else
#define DBG_PRINT(fmt, ...) do {} while(0)
#endif

bool validate_dimension(size_t height, size_t width);

color_t get_pixel(color_t* grid, uint32_t x, uint32_t y, uint32_t width);
color_t* get_pixel_addr(color_t* grid, uint32_t x, uint32_t y, uint32_t width);
void set_pixel(color_t* grid, color_t value, uint32_t x, uint32_t y, uint32_t width);
#endif