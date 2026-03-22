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

color_t get_pixel(color_t* grid, int x, int y, int width);
void set_pixel(color_t* grid, color_t value, int x, int y, int width);
#endif