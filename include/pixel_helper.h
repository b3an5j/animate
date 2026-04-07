/**
 * @file physics_helper.h
 * @brief Pixel and pixel grid helper
*/
#ifndef PIXEL_HELPER_H
#define PIXEL_HELPER_H

#include "animate.h"
#include <sys/types.h>

/**
 * @brief Represents little endian ARGB pixel as
 * a raw 32-bit value word or
 * individual ARGB components in a struct.
 * Assumed Little-Endian ARGB.
 */
typedef union {
    color_t raw;    /**< Raw color value (0xBBGGRRAA) */
    struct {
        uint8_t B;  /**< Blue channel */
        uint8_t G;  /**< Green channel */
        uint8_t R;  /**< Red channel */
        uint8_t A;  /**< Alpha channel */
    } colors;       /**< Named color components */
} Channel;

/**
 * @brief Enum for pixel coloring
 */
typedef enum {
    NORMAL,     /**< Leave alpha channel value as is */
    FORCE_ALPHA /**< Force non-zero alpha to be 0xFF */
} ColorMode;


/**
 * @brief Get the address of a pixel (x, y) in a pixel grid.
 *
 * @param grid The pixel grid in question
 * @param x x coordinate
 * @param y y coordinate
 * @param height Height of the grid
 * @param width Width of the grid
 * @return The address of the pixel
 */
static inline color_t* pixel_get_addr(color_t* grid,
                                      ssize_t x, ssize_t y,
                                      size_t height, size_t width)
{
    return &grid[x + (y * width)];
}

/**
 * @brief Get the color of a pixel (x, y) in a pixel grid.
 *
 * @param grid The pixel grid in question
 * @param x x coordinate
 * @param y y coordinate
 * @param height Height of the grid
 * @param width Width of the grid
 * @return The color of the pixel
 */
static inline color_t pixel_get_color(color_t* grid,
                                      ssize_t x, ssize_t y,
                                      size_t height, size_t width)
{
    return *pixel_get_addr(grid, x, y, height, width);
}

/**
 * @brief Set color of a pixel (x, y) in a pixel grid.
 *
 * @param grid The pixel grid in question
 * @param value The color value to be set
 * @param x x coordinate
 * @param y y coordinate
 * @param height Height of the grid
 * @param width Width of the grid
 * @param mode ColorMode enum
 */
static inline void pixel_set_color(color_t* grid,
                                   color_t value,
                                   ssize_t x, ssize_t y,
                                   size_t height, size_t width,
                                   ColorMode mode)
{
    if (mode == FORCE_ALPHA) {
        value |= 0xFF << 24;
    }
    *pixel_get_addr(grid, x, y, height, width) = value;
}

/**
 * @brief Get coordinate for edge warping purpose.
 *
 * @param coord coordinate
 * @param length Max width along the coordinate
 * @return Coordinate in range [0, Max width)
 */
static inline ssize_t pixel_get_positive_coord(ssize_t coord, size_t length)
{
    if (!length) { return 0; }
    ssize_t res = coord % (ssize_t)length;
    return (res < 0) ? (res + (ssize_t)length) : res;
}

#endif /* PIXEL_HELPER_H */