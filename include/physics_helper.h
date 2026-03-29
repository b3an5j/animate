#ifndef PHYSICS_HELPER_H
#define PHYSICS_HELPER_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "misc_helper.h"

struct params;

struct params* physics_create_params();
void physics_set_params(struct params* params,
    ssize_t initial_x, ssize_t initial_y,
    ssize_t x_velocity, ssize_t y_velocity,
    ssize_t x_acceleration, ssize_t y_acceleration);
ssize_t physics_calculate_posx(ssize_t initial_x, struct params* params, double delta_time);
ssize_t physics_calculate_posy(ssize_t initial_y, struct params* params, double delta_time);
double physics_get_deltatime(size_t frame, size_t frame_rate);

#endif