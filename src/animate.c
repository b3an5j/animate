#include "animate.h"
#include "canvas_helper.h"
#include "misc_helper.h"
#include "physics_helper.h"
#include "pixel_helper.h"

struct sprite {
    size_t height;
    size_t width;
    color_t* grid;
};

struct sprite_placement {
    ssize_t initial_x;
    ssize_t initial_y;
    struct sprite* sprite;
    struct list_node* listnode;
    struct params* params;
};

struct canvas {
    size_t height;
    size_t width;
    color_t* grid;
    struct circular_list* layers;
};


/* This is the header structure that we can expect to find at position 0 in a bitmap file. */
struct bitmap_header {
    uint8_t  magic[2];          // Expect {'B', 'M'}
    uint32_t size_bytes;        // Size of the file in bytes
    uint16_t reserved[2];
    uint32_t pixel_offset;      // Starting address of pixel data
    // Don't pad this struct for alignment
} __attribute__((packed));

/*
 * This header immediately follows bitmap_header in the file.
 * You may ignore all fields except bV5Width and bV5Height, unless you'd like to validate
 * the image format
 */
struct bitmapv5_header {
    // Offset     Size Description
    uint32_t bV5Size; // 0x00          4 Size of this header (124 bytes)
    uint32_t bV5Width; // 0x04          4 Width of the bitmap in pixels
    uint32_t bV5Height; // 0x08          4 Height of the bitmap in pixels
    uint16_t bV5Planes; // 0x0C          2 Number of planes (must be 1)
    uint16_t bV5BitCount; // 0x0E          2 Bits per pixel (e.g., 32)
    uint32_t bV5Compression; // 0x10          4 BI_RGB (0), BI_BITFIELDS (3)
    uint32_t bV5SizeImage; // 0x14          4 Size of image data (0 if uncompressed)
    uint32_t bV5XPelsPerMeter; // 0x18          4 Horizontal pixels per meter
    uint32_t bV5YPelsPerMeter; // 0x1C          4 Vertical pixels per meter
    uint32_t bV5ClrUsed; // 0x20          4 Number of color indices used
    uint32_t bV5ClrImportant; // 0x24          4 Number of important colors
    uint32_t bV5RedMask; // 0x28          4 Color mask for red component
    uint32_t bV5GreenMask; // 0x2C          4 Color mask for green component
    uint32_t bV5BlueMask; // 0x30          4 Color mask for blue component
    uint32_t bV5AlphaMask; // 0x34          4 Color mask for alpha channel
    uint32_t bV5CSType; // 0x38          4 Color space type (e.g., LCS_CALIBRATED_RGB)
    uint8_t  bV5Endpoints[36]; // 0x3C-0x5B    36 CIE XYZ color space endpoints
    uint32_t bV5GammaRed; // 0x5C          4 Gamma red component
    uint32_t bV5GammaGreen; // 0x60          4 Gamma green component
    uint32_t bV5GammaBlue; // 0x64          4 Gamma blue component
    uint32_t bV5Intent; // 0x68          4 Rendering intent
    uint32_t bV5ProfileData; // 0x6C          4 Offset to ICC profile data
    uint32_t bV5ProfileSize; // 0x70          4 Size of embedded profile data
    uint32_t bV5Reserved; // 0x74          4 Reserved (must be 0)
};
/* height and width should be int32_t instead */


struct canvas* animate_create_canvas(size_t height, size_t width,
    color_t background_color)
{
    if (!validate_dimension(height, width)) {
        return NULL;
    }

    struct canvas* cloth = malloc(sizeof(*cloth));
    if (!cloth) {
        DBG_PRINT(ERR_MALLOC, "canvas");
        return NULL;
    }
    cloth->height = height;
    cloth->width = width;
    cloth->grid = NULL; // careful
    cloth->layers = canvas_create_circularlist();
    if (!cloth->layers) {
        goto fail_canvas;
    }

    // just to be sure
    size_t total_pixel = height * width;
    if (total_pixel > (SIZE_MAX / sizeof(color_t))) {
        DBG_PRINT(TOOBIG, "Canvas");
        goto fail_canvas;
    }

    cloth->grid = malloc(total_pixel * sizeof(color_t));
    if (!cloth->grid) {
        DBG_PRINT(ERR_MALLOC_WSIZE, "canvas grid",
            cloth->height, cloth->width);
        goto fail_canvas;
    }

    // color cloth
    for (size_t i = 0; i < total_pixel; ++i) {
        cloth->grid[i] = background_color;
    }
    DBG_PRINT(SUCCESS, "Create canvas");
    return cloth;

fail_canvas:
    DBG_PRINT(FAIL, "Create canvas");
    animate_destroy_canvas(cloth);
    return NULL;
}

struct sprite* animate_create_sprite(const char* file)
{
    size_t ret; // temp for fread

    /* GET SPRITE INFO */
    FILE* fp = fopen(file, "rb");
    if (!fp) {
        DBG_PRINT(ERR_IO, file);
        return NULL;
    }

    // get header
    struct bitmap_header header_bmp;
    ret = fread(&header_bmp, sizeof(header_bmp), 1, fp);
    if (ret != 1) {
        DBG_PRINT(ERR_BM_HEADER, file);
        goto fail_header;
    }

    // then get v5 header
    uint32_t v5_size;
    struct bitmapv5_header header_v5;
    ret = fread(&v5_size, sizeof(uint32_t), 1, fp);
    if (ret != 1) {
        DBG_PRINT(CUSTOM, "Failed to read header size of %s\n", file);
        goto fail_header;
    }
    // verify size
    if (v5_size != 124) {
        DBG_PRINT(CUSTOM, "BITMAPV5 size of %s is %u not 124\n", file, v5_size);
        goto fail_header;
    }
    // get all v5 header info
    fseek(fp, -(long)sizeof(uint32_t), SEEK_CUR);
    ret = fread(&header_v5, sizeof(header_v5), 1, fp);
    if (ret != 1) {
        DBG_PRINT(ERR_BM_V5, file);
        goto fail_header;
    }

    /* INITIALISE SPRITE */
    struct sprite* sprite = malloc(sizeof(*sprite));
    if (!sprite) {
        DBG_PRINT(ERR_MALLOC, "sprite");
        return NULL;
    }
    sprite->grid = NULL; // careful
    if ((sprite->height = header_v5.bV5Height) == 0\
        || (sprite->width = header_v5.bV5Width) == 0) {
        DBG_PRINT(INVALID_DIM, header_v5.bV5Height, header_v5.bV5Width);
        goto fail_sprite;
    }
    sprite->height = header_v5.bV5Height;
    sprite->width = header_v5.bV5Width;
    size_t total_pixels = header_v5.bV5Height * header_v5.bV5Width;
    sprite->grid = NULL; // careful

    sprite->grid = malloc(total_pixels * sizeof(color_t));
    if (!sprite->grid) {
        DBG_PRINT(ERR_MALLOC_WSIZE, "sprite grid",
            sprite->height, sprite->width);
        goto fail_sprite;
    }

    fseek(fp, header_bmp.pixel_offset, SEEK_SET);
    // flip bitmap
    for (size_t y = 0; y < sprite->height; ++y) {
        size_t y_flipped = (sprite->height - 1) - y;

        for (size_t x = 0; x < sprite->width; ++x) {
            color_t pixel;
            ret = fread(&pixel, sizeof(color_t), 1, fp);
            if (ret != 1) {
                DBG_PRINT(CUSTOM, "Failed to read pixels of %s\n", file);
                goto fail_sprite;
            }

            pixel_set_color(
                sprite->grid,
                pixel,
                x,
                y_flipped,
                sprite->height,
                sprite->width,
                NORMAL
            );
        }
    }

    fclose(fp);
    DBG_PRINT(SUCCESS, "Create sprite");
    return sprite;

fail_header:
    fclose(fp);
    DBG_PRINT(FAIL, "Create sprite");
    return NULL;

fail_sprite:
    animate_destroy_sprite(sprite);
    fclose(fp);
    DBG_PRINT(FAIL, "Create sprite");
    return NULL;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled)
{
    /* INITIALISE SPRITE */
    struct sprite* circle = malloc(sizeof(*circle));
    if (!circle) {
        DBG_PRINT(ERR_MALLOC, "circle sprite");
        return NULL;
    }

    /* SPECIAL CASE */
    if (!radius) {
        circle->height = 1;
        circle->width = 1;
        circle->grid = NULL; // careful

        circle->grid = malloc(sizeof(color_t));
        if (!circle->grid) {
            DBG_PRINT(ERR_MALLOC_WSIZE, "circle grid", diameter, diameter);
            goto fail_circle;
        }

        // exit early
        *(circle->grid) = c;
        return circle;
    }

    /* NORMAL CASE */
    size_t diameter = (2 * radius) - 1;
    if (!validate_dimension(diameter, diameter)) {
        goto fail_circle;
    }

    circle->height = diameter;
    circle->width = diameter;
    circle->grid = NULL; // careful

    circle->grid = calloc(diameter * diameter, sizeof(color_t));
    if (!circle->grid) {
        DBG_PRINT(ERR_MALLOC_WSIZE, "circle grid", radius, radius);
        goto fail_circle;
    }


    /* CALCULATE THE BOUNDARY AND FILL COLOR */
    // assume start from middle, all diameter odd, easier to place
    // if not filled, color only edges
    // Midpoint functions are scaled by 4
    uint32_t x = 0;
    uint32_t y = radius - 1;
    uint32_t offset = radius - 1;
    // x = 0, y = radius, initial d is (scaled) 5-4r
    int32_t d = 5 - (4 * radius);

    while (x <= y) { // as y decreases, will meet when 45 degrees
        // outline only
        pixel_set_color(circle->grid, c, (offset + y), (offset + x), diameter, diameter, NORMAL); // octant 1
        pixel_set_color(circle->grid, c, (offset + x), (offset + y), diameter, diameter, NORMAL); // octant 2
        pixel_set_color(circle->grid, c, (offset - x), (offset + y), diameter, diameter, NORMAL); // octant 3
        pixel_set_color(circle->grid, c, (offset - y), (offset + x), diameter, diameter, NORMAL); // octant 4
        pixel_set_color(circle->grid, c, (offset - y), (offset - x), diameter, diameter, NORMAL); // octant 5
        pixel_set_color(circle->grid, c, (offset - x), (offset - y), diameter, diameter, NORMAL); // octant 6
        pixel_set_color(circle->grid, c, (offset + x), (offset - y), diameter, diameter, NORMAL); // octant 7
        pixel_set_color(circle->grid, c, (offset + y), (offset - x), diameter, diameter, NORMAL); // octant 8

        // fill left to right
        if (filled) {
            for (int32_t head = (offset - x) + 1; head < (offset + x); ++head) {
                pixel_set_color(circle->grid, c, head, offset + y, diameter, diameter, NORMAL);
                pixel_set_color(circle->grid, c, head, offset - y, diameter, diameter, NORMAL);
            }
            for (int32_t head = (offset - y) + 1; head < (offset + y); ++head) {
                pixel_set_color(circle->grid, c, head, offset + x, diameter, diameter, NORMAL);
                pixel_set_color(circle->grid, c, head, offset - x, diameter, diameter, NORMAL);
            }
        }

        // d_step = d_next - d
        if (d < 0) {
            // Y NO CHANGE
            // d = x^2 + (y + .5)^2 - r^2
            // d_next = (x+1)^2 + (y + .5)^2 - r^2
            // d_step = 2x + 1 or scaled 8x + 4
            d += 8 * x + 4;
        }
        else {
            // Y CHANGE
            // d = x^2 + (y - .5)^2 - r^2
            // d_next = (x+1)^2 + (y - 1.5)^2 - r^2
            // d_step = 2(x-y) + 3 or scaled 8(x-y) + 12
            d += 8 * (x - y) + 12;
            --y;
        }
        ++x;
    }

    DBG_PRINT(SUCCESS, "Create sprite");
    return circle;

fail_circle:
    animate_destroy_sprite(circle);
    DBG_PRINT(FAIL, "Create circle");
    return NULL;
}

struct sprite* animate_create_rectangle(size_t width, size_t height,
    color_t c, bool filled)
{
    if (!validate_dimension(height, width)) {
        return NULL;
    }

    /* INITIALISE SPRITE */
    struct sprite* rect = malloc(sizeof(*rect));
    if (!rect) {
        DBG_PRINT(ERR_MALLOC, "rectangle sprite");
        return NULL;
    }
    rect->height = height;
    rect->width = width;
    size_t total_pixels = width * height;
    rect->grid = NULL; // careful

    rect->grid = calloc(height * width, sizeof(color_t));
    if (!rect->grid) {
        DBG_PRINT(ERR_MALLOC_WSIZE, "rectangle grid",
            rect->height, rect->width);
        goto fail_rectangle;
    }

    /* FILL COLOR */
    // fill all
    if (filled) {
        color_t* head = rect->grid;
        for (size_t i = 0; i < total_pixels; ++i) {
            head[i] = c;
        }
    }
    // outline
    else {
        // bottom and up
        for (uint32_t x = 0; x < rect->width; ++x) {
            pixel_set_color(rect->grid, c, x, 0,
                rect->height, rect->width, NORMAL);
            pixel_set_color(rect->grid, c, x, rect->height - 1,
                rect->height, rect->width, NORMAL);
        }
        // left and right
        for (uint32_t y = 1; y < rect->height - 1; ++y) {
            pixel_set_color(rect->grid, c, 0, y,
                rect->height, rect->width, NORMAL);
            pixel_set_color(rect->grid, c, rect->width - 1, y,
                rect->height, rect->width, NORMAL);
        }
    }

    DBG_PRINT(SUCCESS, "Create sprite");
    return rect;

fail_rectangle:
    animate_destroy_sprite(rect);
    DBG_PRINT(FAIL, "Create rectangle");
    return NULL;
}

bool animate_destroy_sprite(struct sprite* sprite)
{
    if (!sprite) {
        DBG_PRINT(FREED, "Nothing / NULL");
        return false;
    }

    free(sprite->grid);
    sprite->grid = NULL; // careful
    free(sprite);
    DBG_PRINT(FREED, "Sprite");
    return false;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas,
    struct sprite* sprite,
    ssize_t x, ssize_t y)
{
    if (!canvas) {
        DBG_PRINT(INVALID_ARG, "canvas");
        return NULL;
    }
    if (!sprite) {
        DBG_PRINT(INVALID_ARG, "sprite");
        return NULL;
    }

    /* INITIALISE SPRITE PLACEMENT */
    struct sprite_placement* placement = malloc(sizeof(*placement));
    if (!placement) {
        DBG_PRINT(ERR_MALLOC, "sprite placement");
        return NULL;
    }
    placement->initial_x = x;
    placement->initial_y = y;
    placement->sprite = sprite;
    placement->listnode = circularlist_insert(canvas->layers, placement, TOP);
    placement->params = physics_create_params();
    return placement;
}

void animate_placement_up(struct sprite_placement* sprite_placement)
{
    circularlist_move(
        listnode_get_thislist(sprite_placement->listnode),
        sprite_placement->listnode,
        MOVEUP
    );
    DBG_PRINT(CUSTOM, "Moved sprite UP");
}

void animate_placement_down(struct sprite_placement* sprite_placement)
{
    circularlist_move(
        listnode_get_thislist(sprite_placement->listnode),
        sprite_placement->listnode,
        MOVEDOWN
    );
    DBG_PRINT(CUSTOM, "Moved sprite DOWN");
}

void animate_placement_top(struct sprite_placement* sprite_placement)
{
    circularlist_move(
        listnode_get_thislist(sprite_placement->listnode),
        sprite_placement->listnode,
        TOP
    );
    DBG_PRINT(CUSTOM, "Moved sprite TOP");
}

void animate_placement_bottom(struct sprite_placement* sprite_placement)
{
    circularlist_move(
        listnode_get_thislist(sprite_placement->listnode),
        sprite_placement->listnode,
        BOTTOM
    );
    DBG_PRINT(CUSTOM, "Moved sprite BOTTOM");
}

void animate_destroy_placement(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement) {
        DBG_PRINT(FREED, "Nothing / NULL");
        return;
    }

    physics_destroy_params(sprite_placement->params);
    circularlist_remove(
        sprite_placement->listnode,
        listnode_get_thislist(sprite_placement->listnode)
    );
    free(sprite_placement);
    DBG_PRINT(FREED, "Sprite placement");
}

void animate_set_animation_params(struct sprite_placement* sprite_placement,
    ssize_t vx, ssize_t vy,
    ssize_t ax, ssize_t ay)
{
    physics_set_params(
        sprite_placement->params,
        sprite_placement->initial_x, sprite_placement->initial_y,
        vx, vy,
        ax, ay
    );
}

void animate_destroy_canvas(struct canvas* canvas)
{
    if (!canvas) {
        DBG_PRINT(FREED, "Nothing / NULL");
        return;
    }
    canvas_destroy_circularlist(canvas->layers);
    free(canvas->grid);
    free(canvas);
    DBG_PRINT(FREED, "Canvas");
}

size_t animate_frame_size_bytes(struct canvas* canvas)
{
    return canvas->height * canvas->width * sizeof(color_t);
}

void animate_generate_frame(const struct canvas* canvas, size_t frame,
    size_t frame_rate, void* buf)
{
    /* COLOR BACKGROUND */
    for (size_t pixel_y = 0; pixel_y < canvas->height; ++pixel_y) {
        for (size_t pixel_x = 0; pixel_x < canvas->width; ++pixel_x) {
            pixel_set_color(
                (color_t*)buf,
                pixel_get_color(canvas->grid, pixel_x, pixel_y,
                    canvas->height, canvas->width),
                pixel_x,
                pixel_y,
                canvas->height,
                canvas->width,
                FORCE_ALPHA
            );
        }
    }

    /* COLOR ALL SPRITE PLACEMENTS */
    circularlist_goto_first(canvas->layers); // reset
    double deltatime = physics_get_deltatime(frame, frame_rate);

    /* GO THROUGH EVERY LAYER */
    for (uint32_t i = 0; i < circularlist_get_listsize(canvas->layers); ++i) {
        struct sprite_placement* layer = canvas_advance_layer(canvas->layers);
        struct sprite* thesprite = layer->sprite;
        struct params* params = layer->params;
        ssize_t posx = physics_calculate_posx(
            layer->initial_x,
            params,
            deltatime
        );
        ssize_t posy = physics_calculate_posy(
            layer->initial_y,
            params,
            deltatime
        );

        /* Y OF SPRITE */
        for (size_t pixel_y = 0; pixel_y < thesprite->height; ++pixel_y) {
            ssize_t target_y = posy + (ssize_t)pixel_y;

            /* X OF SPRITE */
            for (size_t pixel_x = 0; pixel_x < thesprite->width; ++pixel_x) {
                ssize_t target_x = posx + (ssize_t)pixel_x;

                // out of bounds
                if (target_x < 0 || target_x >= (ssize_t)canvas->width ||
                    target_y < 0 || target_y >= (ssize_t)canvas->height) {
                    continue;
                }

                Channel c;
                c.raw = pixel_get_color(thesprite->grid, pixel_x, pixel_y,
                    thesprite->height, thesprite->width);
                if (!c.colors.A) { // alpha is 0
                    continue;
                }

                pixel_set_color(
                    (color_t*)buf,
                    c.raw,
                    target_x,
                    target_y,
                    canvas->height,
                    canvas->width,
                    FORCE_ALPHA
                );
            }
        }
    }
}

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement,
    animate_fn, void* priv)
{
}