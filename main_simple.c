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

#define OUTPUT_FILE "imgs/simple.dat"

int main(int argc, char** argv)
{
    struct canvas* canvas = animate_create_canvas(600, 800, animate_color_argb(255, 255, 255, 255));
    struct sprite* rect = animate_create_rectangle(50, 50, animate_color_argb(255, 255, 255, 0), 1);

    struct sprite* rick = animate_create_sprite("imgs/rick.bmp");
    struct sprite* crosshair = animate_create_sprite("imgs/crosshair.bmp");

    struct sprite_placement* prect1 = animate_place_sprite(canvas, rect, 100, 100);
    struct sprite_placement* prect2 = animate_place_sprite(canvas, rect, 0, 0);
    struct sprite_placement* prick1 = animate_place_sprite(canvas, rick, 0, 0);
    struct sprite_placement* pch1 = animate_place_sprite(canvas, crosshair, 0, 0);
    if (!prect1 || !prect2 || !prick1 || !pch1) {
        printf("FAILED\n");
    }
    animate_placement_top(prect1);

    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size_bytes);
    animate_generate_frame(canvas, 1, 25, data);

    FILE* fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("Failed to open target file\n");
        return -1;
    }

    size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
    if (bytes_written != frame_size_bytes) {
        printf("Failed to write buffer (%ld/%ld): %s\n", bytes_written,
            frame_size_bytes, strerror(errno));
        return -1;
    }

    fclose(fp);
    free(data);

    animate_destroy_canvas(canvas);
    animate_destroy_sprite(rect);
    animate_destroy_sprite(rick);
    animate_destroy_sprite(crosshair);

    return 0;
}

