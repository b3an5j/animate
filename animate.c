#include "animate.h"
#include "helper.h"

#include <stdio.h>
#include <stdlib.h>

struct sprite {
    uint32_t height;
    uint32_t width;
    uint32_t total_pixels;
    color_t* grid;
    struct sprite_placement pos;
};

struct sprite_placement {
    /* -1 is not in use */
    uint32_t x;
    uint32_t y;
};
/* height and width should be int32_t instead */

struct canvas {
    uint32_t height;
    uint32_t width;
    size_t total_pixels;
    color_t* grid;
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
        DBG_PRINT("Failed to allocate memory for %s\n", "canvas");
        return NULL;
    }
    cloth->grid = NULL; // careful
    cloth->height = height;
    cloth->width = width;
    cloth->total_pixels = height * width;
    // just to be sure
    if (cloth->total_pixels > (SIZE_MAX / sizeof(color_t))) {
        DBG_PRINT("Canvas is too big\n%s", "");
        goto fail_canvas;
    }

    cloth->grid = malloc(cloth->total_pixels * sizeof(color_t));
    if (!cloth->grid) {
        DBG_PRINT("Failed to allocate memory for\
            %s %ux%u\n", "canvas grid", cloth->height, cloth->width);
        goto fail_canvas;
    }

    // color cloth
    for (size_t i = 0; i < cloth->total_pixels; ++i) {
        cloth->grid[i] = background_color;
    }
    return cloth;

fail_canvas:
    animate_destroy_canvas(cloth);
    return NULL;
}

struct sprite* animate_create_sprite(const char* file)
{
    size_t ret; // temp for fread

    /* GET SPRITE INFO */
    FILE* fp = fopen(file, "rb");
    if (!fp) {
        DBG_PRINT("Failed to open %s\n", file);
        return NULL;
    }

    // get header
    struct bitmap_header header_bmp;
    ret = fread(&header_bmp, sizeof(header_bmp), 1, fp);
    if (ret != 1) {
        DBG_PRINT("Failed to read bitmap header of %s\n", file);
        goto fail_header;
    }

    // then get v5 header
    uint32_t v5_size;
    struct bitmapv5_header header_v5;
    ret = fread(&v5_size, sizeof(uint32_t), 1, fp);
    if (ret != 1) {
        DBG_PRINT("Failed to read header size of %s\n", file);
        goto fail_header;
    }
    // verify size
    if (v5_size != 124) {
        DBG_PRINT("BITMAPV5 size of %s is %u not 124\n", file, v5_size);
        goto fail_header;
    }
    // get all v5 header info
    fseek(fp, -(long)sizeof(uint32_t), SEEK_CUR);
    ret = fread(&header_v5, sizeof(header_v5), 1, fp);
    if (ret != 1) {
        DBG_PRINT("Failed to read v5 header of %s\n", file);
        goto fail_header;
    }

    /* INITIALISE SPRITE */
    struct sprite* spt = malloc(sizeof(*spt));
    if (!spt) {
        DBG_PRINT("Failed to allocate memory for %s %s\n", "sprite", file);
        return NULL;
    }
    spt->grid = NULL; // careful
    if ((spt->height = header_v5.bV5Height) == 0\
        || (spt->width = header_v5.bV5Width) == 0) {
        DBG_PRINT("Invalid dimension, H = %d W = %d\n", \
            header_v5.bV5Height, header_v5.bV5Width);
        goto fail_sprite;
    }
    spt->height = header_v5.bV5Height;
    spt->width = header_v5.bV5Width;
    spt->total_pixels = header_v5.bV5Height * header_v5.bV5Width;
    spt->pos.x = -1;
    spt->pos.y = -1;
    spt->grid = NULL; // careful

    spt->grid = malloc(spt->total_pixels * sizeof(color_t));
    if (!spt->grid) {
        DBG_PRINT("Failed to allocate memory for %s %s %ux%u\n", \
            "sprite grid", file, spt->height, spt->width);
        goto fail_sprite;
    }

    // no need for mask, assumed ARGB32 little endian
    // no need to account for padding
    fseek(fp, header_bmp.pixel_offset, SEEK_SET);
    ret = fread(spt->grid, sizeof(color_t), spt->total_pixels, fp);
    if (ret != spt->total_pixels) {
        DBG_PRINT("Failed to read pixels of %s\n", file);
        goto fail_sprite;
    }

    fclose(fp);
    return spt;

fail_header:
    fclose(fp);
    return NULL;

fail_sprite:
    animate_destroy_sprite(spt);
    fclose(fp);
    return NULL;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled)
{
    // assume point is middle of pixel
    size_t diameter = (2 * radius) - 1;
    if (!validate_dimension(diameter, diameter)) {
        return NULL;
    }

    /* INITIALISE SPRITE */
    struct sprite* circle = malloc(sizeof(*circle));
    if (!circle) {
        DBG_PRINT("Failed to allocate memory for %s\n", "circle sprite");
        return NULL;
    }
    circle->height = diameter;
    circle->width = diameter;
    circle->total_pixels = diameter * diameter;
    circle->pos.x = -1;
    circle->pos.y = -1;
    circle->grid = NULL; // careful

    circle->grid = calloc(diameter * diameter, sizeof(color_t));
    if (!circle->grid) {
        DBG_PRINT("Failed to allocate memory for %s radius %zu\n",
            "circle grid", radius);
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

    color_t* octant1, * octant2, * octant3, * octant4, * octant5, * octant6, * octant7, * octant8;
    while (x <= y) { // as y decreases, will meet when 45 degrees
        octant1 = get_pixel_addr(circle->grid, (offset + y), (offset + x), diameter); // octant 1
        octant2 = get_pixel_addr(circle->grid, (offset + x), (offset + y), diameter); // octant 2
        octant3 = get_pixel_addr(circle->grid, (offset - x), (offset + y), diameter); // octant 3
        octant4 = get_pixel_addr(circle->grid, (offset - y), (offset + x), diameter); // octant 4
        octant5 = get_pixel_addr(circle->grid, (offset - y), (offset - x), diameter); // octant 5
        octant6 = get_pixel_addr(circle->grid, (offset - x), (offset - y), diameter); // octant 6
        octant7 = get_pixel_addr(circle->grid, (offset + x), (offset - y), diameter); // octant 7
        octant8 = get_pixel_addr(circle->grid, (offset + y), (offset - x), diameter); // octant 8

        // outline
        *octant1 = *octant2 = *octant3 = *octant4 = *octant5 = *octant6 = *octant7 = *octant8 = c;
        // fill left to right, right to left
        if (filled) {
            color_t* head;
            head = octant3 + 1; // 3 with 2
            while (head < octant2) { *(head++) = c; }
            head = octant4 + 1; // 4 with 1
            while (head < octant1) { *(head++) = c; }
            head = octant5 + 1; // 5 with 8
            while (head < octant8) { *(head++) = c; }
            head = octant6 + 1;// 6 with 7
            while (head < octant7) { *(head++) = c; }
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

    return circle;

fail_circle:
    animate_destroy_sprite(circle);
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
        DBG_PRINT("Failed to allocate memory for %s\n", "rectangle sprite");
        return NULL;
    }
    rect->height = height;
    rect->width = width;
    rect->total_pixels = width * height;
    rect->pos.x = -1;
    rect->pos.y = -1;
    rect->grid = NULL; // careful

    rect->grid = calloc(height * width, sizeof(color_t));
    if (!rect->grid) {
        DBG_PRINT("Failed to allocate memory for %s %ux%u\n",
            "rectangle grid", rect->height, rect->width);
        goto fail_rectangle;
    }

    /* FILL COLOR */
    // fill all
    if (filled) {
        color_t* head = rect->grid;
        for (size_t i = 0; i < rect->total_pixels; ++i) {
            head[i] = c;
        }
    }
    // outline
    else {
        // bottom and up
        for (uint32_t x = 0; x < rect->width; ++x) {
            set_pixel(rect->grid, c, x, 0, rect->width);
            set_pixel(rect->grid, c, x, rect->height - 1, rect->width);
        }
        // left and right
        for (uint32_t y = 1; y < rect->height - 1; ++y) {
            set_pixel(rect->grid, c, 0, y, rect->width);
            set_pixel(rect->grid, c, rect->width - 1, y, rect->width);
        }
    }

    return rect;

fail_rectangle:
    animate_destroy_sprite(rect);
    return NULL;
}

bool animate_destroy_sprite(struct sprite* sprite)
{
    if (!sprite) {
        DBG_PRINT("%s is freed\n", "Nothing / NULL");
        return false;
    }

    if (sprite->pos.x != -1 && sprite->pos.y != -1) {
        DBG_PRINT("Sprite still in use, position (%u,%u)\n",
            sprite->pos.x, sprite->pos.y);
        return false;
    }
    free(sprite->grid);
    free(sprite);
    DBG_PRINT("%s %ux%u is freed\n", "Sprite",
        sprite->height, sprite->width);
    return true;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas,
    struct sprite* sprite,
    ssize_t x, ssize_t y)
{
    // TODO
    return NULL;
}

void animate_placement_up(struct sprite_placement* sprite_placement)
{
    // TODO COMP9017
}

void animate_placement_down(struct sprite_placement* sprite_placement)
{
    // TODO COMP9017
}

void animate_placement_top(struct sprite_placement* sprite_placement)
{
    // TODO
}

void animate_placement_bottom(struct sprite_placement* sprite_placement)
{
    // TODO
}

void animate_destroy_placement(struct sprite_placement* sprite_placement)
{
    // TODO
}

void animate_set_animation_params(struct sprite_placement* sprite_placement,
    ssize_t vx, ssize_t vy,
    ssize_t ax, ssize_t ay)
{
    // TODO
}

void animate_destroy_canvas(struct canvas* canvas)
{
    if (canvas) {
        free(canvas->grid);
        free(canvas);
        DBG_PRINT("Canvas is freed\n%s", "");
        return;
    }
    DBG_PRINT("Nothing / NULL is freed\n%s", "");
}

size_t animate_frame_size_bytes(struct canvas* canvas)
{
    // TODO
    return 0;
}

void animate_generate_frame(const struct canvas* canvas, size_t frame,
    size_t frame_rate, void* buf)
{
    // TODO
}

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement,
    animate_fn, void* priv)
{
}

