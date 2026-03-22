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
    // a bit dangerous since height and width are size_t, safety check?
    if (!height || !width || height > UINT32_MAX || width > UINT32_MAX) {
        DBG_PRINT("Invalid dimension, H = %zu W = %zu\n", height, width);
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
        goto fail_sprite;
    }
    spt->grid = NULL; // careful
    if ((spt->height = header_v5.bV5Height) == 0\
        || (spt->width = header_v5.bV5Width) == 0) {
        DBG_PRINT("Invalid dimension, H = %d W = %d\n", \
            header_v5.bV5Height, header_v5.bV5Width);
        goto fail_sprite;
    }
    spt->pos.x = -1;
    spt->pos.y = -1;

    // avoid SizeImage 0
    spt->total_pixels = header_v5.bV5Height * header_v5.bV5Width;
    spt->grid = malloc(spt->total_pixels * sizeof(color_t));
    if (!spt->grid) {
        DBG_PRINT("Failed to allocate memory for %s %s\n", \
            "sprite grid", file);
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

    fclose(fp);
    return NULL;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled)
{
    struct sprite* circle = malloc(sizeof(*circle));
    if (!circle) {
        DBG_PRINT("Failed to allocate memory for %s\n", "circle sprite");
        return NULL;
    }

    /* CALCULATE THE BOUNDARY AND FILL */
    // since we only deal with radius, no odd diameter
    // if not filled, color only edges
    size_t x = 0;
    size_t y = radius;
    while (x < y) {
        ++x;
    }

fail_circle:
    animate_destroy_sprite(circle);
    return NULL;
}

struct sprite* animate_create_rectangle(size_t width, size_t height,
    color_t c, bool filled)
{
    // TODO
    return NULL;
}

bool animate_destroy_sprite(struct sprite* sprite)
{
    if (!sprite) {
        DBG_PRINT("Nothing / NULL is freed\n%s", "");
        return 0;
    }

    if ((sprite->pos).x != -1) {
        DBG_PRINT("Sprite still in use\n%s", "");
        return 1;
    }
    free(sprite->grid);
    free(sprite);
    DBG_PRINT("Sprite is freed\n%s", "");
    return 0;
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

