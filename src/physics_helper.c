#include "physics_helper.h"

struct direction {
    ssize_t velocity;
    ssize_t acceleration;
};

struct params {
    struct direction x;
    struct direction y;
};

struct params* physics_create_params()
{
    struct params* params = malloc(sizeof(*params));
    if (!params) {
        DBG_PRINT(ERR_MALLOC, "params");
        DBG_PRINT(FAIL, "Create params");
        return NULL;
    }

    physics_set_params(
        params,
        0, 0,
        0, 0,
        0, 0
    );
    DBG_PRINT(SUCCESS, "Create params");
    return params;
}

void physics_destroy_params(struct params* params)
{
    free(params);
}

void physics_set_params(
    struct params* params,
    ssize_t initial_x, ssize_t initial_y,
    ssize_t x_velocity, ssize_t y_velocity,
    ssize_t x_acceleration, ssize_t y_acceleration)
{
    params->x = (struct direction){
        .velocity = x_velocity,
        .acceleration = x_acceleration
    };
    params->y = (struct direction){
        .velocity = y_velocity,
        .acceleration = y_acceleration
    };
}

static ssize_t physics_calculate_posdiff(struct direction* dir, double delta_time)
{
    double v_diff = (dir->velocity) ? (double)dir->velocity * delta_time : 0;
    double a_diff = (dir->acceleration) ?
        0.5 * (double)dir->acceleration * delta_time * delta_time :
        0;
    double displacement = v_diff + a_diff;
    // rounding hack
    displacement = (displacement >= 0) ? displacement + 0.5 : displacement - 0.5;
    return (ssize_t)displacement;
}

ssize_t physics_calculate_posx(ssize_t initial_x, struct params* params, double delta_time)
{
    return initial_x + physics_calculate_posdiff(&params->x, delta_time);
}

ssize_t physics_calculate_posy(ssize_t initial_y, struct params* params, double delta_time)
{
    return initial_y + physics_calculate_posdiff(&params->y, delta_time);
}

double physics_get_deltatime(size_t frame, size_t frame_rate)
{
    if (frame <= 0) {
        return 0;
    }
    return (double)frame / (double)frame_rate;
}