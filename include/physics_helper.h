/**
 * @file physics_helper.h
 * @brief Physics calculations
*/
#ifndef PHYSICS_HELPER_H
#define PHYSICS_HELPER_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include "misc_helper.h"

/**
 * @struct params
 * @brief Stores the movement parameters of the sprite,
 * i.e. initial x y velocity,
 * and x y acceleration given by user.
 */
struct params;


/**
 * @brief Creates an empty parameter storage.
 *
 * @return The new parameter storage
 */
struct params* physics_create_params();

/**
 * @brief Destroy a parameter storage.
 *
 * @param params The parameter storage to be destroyed
 */
void physics_destroy_params(struct params* params);


/**
 * @brief Sets the parameters given by user.
 *
 * @param params The parameter storage
 * @param initial_x initial x position
 * @param initial_y initial y position
 * @param x_velocity initial x velocity
 * @param y_velocity initial y velocity
 * @param x_acceleration x acceleration
 * @param y_acceleration y acceleration
 */
void physics_set_params(struct params* params,
                        ssize_t initial_x, ssize_t initial_y,
                        ssize_t x_velocity, ssize_t y_velocity,
                        ssize_t x_acceleration, ssize_t y_acceleration);


/**
 * @brief Calculates the current x position of a sprite placement.
 *
 * @param initial_x initial x position
 * @param params The parameter storage
 * @param delta_time time passed since t0 (first frame)
 * @return Current x position
 */
ssize_t physics_calculate_posx(ssize_t initial_x,
                               struct params* params,
                               double delta_time);

/**
 * @brief Calculates the current y position of a sprite placement.
 *
 * @param initial_x initial y position
 * @param params The parameter storage
 * @param delta_time time passed since t0 (first frame)
 * @return Current y position
 */
ssize_t physics_calculate_posy(ssize_t initial_y,
                               struct params* params,
                               double delta_time);

/**
 * @brief Calculates time passed since t0 (first frame)
 *
 * @param frame Frame number
 * @param frame_rate Frame rate of the intended animation
 * @return Time passed since t0 (first frame)
 */
double physics_get_deltatime(size_t frame, size_t frame_rate);

#endif /* PHYSICS_HELPER_H */