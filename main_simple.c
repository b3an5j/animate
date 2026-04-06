/**
 * A simple test file to help you get started
 */

#include "animate.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define OUTPUT_FILE "./tests/out/simple.dat"

void output_frames(struct canvas* canvas, size_t n_frames, size_t frame_rate);

int main(int argc, char** argv)
{
    struct canvas* canvas = animate_create_canvas(600, 800, animate_color_argb(255, 100, 255, 200));
    struct sprite* rect = animate_create_rectangle(100, 100, animate_color_argb(255, 255, 255, 0), 1);
    struct sprite* circ = animate_create_circle(80, animate_color_argb(1, 0, 255, 0), 0);

    struct sprite* rick = animate_create_sprite("./tests/imgs/rick.bmp");
    struct sprite* crosshair = animate_create_sprite("./tests/imgs/crosshair.bmp");

    struct sprite_placement* prect1 = animate_place_sprite(canvas, rect, 800, 300);
    struct sprite_placement* prect2 = animate_place_sprite(canvas, rect, 0, 0);
    struct sprite_placement* prick1 = animate_place_sprite(canvas, rick, 100, 100);
    struct sprite_placement* prick2 = animate_place_sprite(canvas, rick, 100, 300);
    struct sprite_placement* pcirc = animate_place_sprite(canvas, circ, 400, 0);
    struct sprite_placement* pch = animate_place_sprite(canvas, crosshair, 0, 0);
    if (!prect1 || !prect2 || !prick1 || !prick2 || !pcirc || !pch) {
        printf("FAILED\n");
        return 1;
    }

    /* TEST LAYERING */
    // BEFORE = (bot) prect1 > prect2 > prick1 > prick2 > pcirc > pch (top)
    animate_placement_bottom(prick1);
    animate_placement_top(prect1);
    animate_placement_up(prick2);
    animate_placement_down(pcirc);
    // AFTER = (bot) prick1 > pcirc > prect2 > prick2 > pch > prect1 (top)

    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size_bytes);
    animate_generate_frame(canvas, 1, 25, data);

    FILE* fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("Failed to open target file");
        return -1;
    }

    size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
    if (bytes_written != frame_size_bytes) {
        printf("Failed to write buffer (%ld/%ld): %s\n", bytes_written,
            frame_size_bytes, strerror(errno));
        return -1;
    }

    /* TEST PHYSICS */
    animate_set_animation_params(prect1, -40, 0, -40, 0);
    animate_set_animation_params(prect2, 100, 300, 0, -120);
    animate_set_animation_params(prick1, 80, 80, 0, 0);
    animate_set_animation_params(prick2, 150, 0, -40, 0);
    animate_set_animation_params(pcirc, 0, 40, 0, 15);
    animate_set_animation_params(pch, 40, 40, 20, 20);
    output_frames(canvas, 300, 60);

    fclose(fp);
    free(data);

    animate_destroy_canvas(canvas);
    animate_destroy_sprite(rect);
    animate_destroy_sprite(rick);
    animate_destroy_sprite(crosshair);
    animate_destroy_sprite(circ);

    return 0;
}

void output_frames(struct canvas* canvas, size_t n_frames, size_t frame_rate)
{
    static const char* const prefix = "tests/frames/frame%06zu.raw";
    char filename[64];

    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size_bytes);
    if (!data) { return; }

    for (size_t frame = 0; frame < n_frames; ++frame) {
        snprintf(filename, 64, prefix, frame);
        animate_generate_frame(canvas, frame, frame_rate, data);

        FILE* fp = fopen(filename, "wb");
        if (fp == NULL) {
            perror("Failed to generate file\n");
            goto clean_data;
        }
        size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
        fclose(fp);

        if (bytes_written != frame_size_bytes) {
            printf("Failed to write buffer (%zu/%zu): %s\n", bytes_written,
                frame_size_bytes, strerror(errno));
            goto clean_data;
            return;
        }
    }

clean_data:
    free(data);
}