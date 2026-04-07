# PointerPro Animate
A simple animation engine. Moves sprites accross the screen, given movement parameters. Currently physics engine only supports parabolic motion.

## Features
**Parabolic physics**\
Arc based movement with x and y coordinates.\
This follows:
$$\mathbf{p} = \mathbf{p}_0 + \mathbf{v}t + \frac{\mathbf{a}t^2}{2}$$

**Sprite management**\
Handling of multiple references to the same sprite.

**Lightweight**\
Minimal dependencies, small size.

**Customisable**\
Adjust your own velocity, gravity, frame rates, and number of frames.

## Requirements
- **C Compiler:** `gcc` or `clang`
- **ImageMagick:** For image and video generating scripts.
- **Doxygen:** (Optional) To generate documentation.

## Quick Start
Example code:
```c
/* simple.c */
#include "animate.h" // make sure header is discoverable!

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define OUTPUT_FILE "simple.dat"

int main(int argc, char* argv[])
{
    // Create a canvas 800x600
    struct canvas* canvas = animate_create_canvas(600, 800, animate_color_argb(255, 100, 255, 200));
    
    // Create a rectangle 100x100
    struct sprite* rect = animate_create_rectangle(100, 100, animate_color_argb(255, 255, 255, 0), 1);
    
    // Place sprite on canvas on (800, 300)
    struct sprite_placement* prect1 = animate_place_sprite(canvas, rect, 300, 300);
    
    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size_bytes);
    // frame 1 (2nd frame), 25 fps
    animate_generate_frame(canvas, 1, 25, data);
    
    // Write to a .dat or .raw file
    FILE* fp = fopen(OUTPUT_FILE, "w");
    if (fp == NULL) {
        perror("Failed to open target file");
        goto cleanup;
    }

    size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
    if (bytes_written != frame_size_bytes) {
        printf("Failed to write buffer (%ld/%ld): %s\n", bytes_written,
            frame_size_bytes, strerror(errno));
        goto cleanup;
    }

cleanup:
    if (fp != NULL) { fclose(fp); }
    free(data);

    animate_destroy_canvas(canvas);
    animate_destroy_sprite(rect);
    return 0;
}
```
Install *imagemagick* if you have not.
Then run:
```bash
gcc animate.o simple.c -o simple # or use clang
./simple # this will generate your .dat file in your current folder

# this will give you out.png in your current folder
./tests/raw_to_img.sh 800 600 ./simple.dat ./out.png
```
More example in `main_simple.c`.\
You can generate the example by running:
```bash
./generate_test.sh
```
More at **Scripts** section

## Scripts
Assuming you are in root dir,

1. `generate_test.sh`\
Usage:
```bash
./generate_test.sh
```
It will generate one-frame png ( `out/out.png` ) and the example animation ( `out/anim.mp4` ) along with its frames ( `frames/` ) in `tests/` folder. The executable `test` of `main_simple.c` is also generated.

2. `tests/animate.sh`\
Usage :
```bash
./tests/animate.sh {FRAMES_FOLDER} {WIDTH} {HEIGHT} {FRATE} {OUTPUT_FILE}
```
`FRAMES_FOLDER` : Path to folder containing frames. One must pad left zero (leading zeros) in order for imagemagick to recognise the numbering.\
`WIDTH` : Width of the canvas\
`HEIGHT` : Height of the canvas\
`FRATE` : Frame rate (fps)\
`OUTPUT_FILE` :  Output file, not only `.mp4`, one may output in `.gif`, etc.\
Animates given frames in frame folder with given fps.

3. `tests/img_to_bmp.sh`\
Usage:
```bash
.tests/img_to_bmp.sh {IMG}
```
`IMG` : The image file you want to convert\
Converts an image into `.bmp` for *animate* to digest.

4. `tests/raw_to_img.sh`\
Usage:
```bash
.tests/raw_to_img.sh {WIDTH} {HEIGHT} {INPUT} {OUTPUT}
```
`WIDTH` : Width of the canvas\
`HEIGHT` : Height of the canvas\
`INPUT` : The raw `.raw` or `.dat` file (frame) you want to convert\
`OUTPUT_FILE` :  Output file, not only `.png`, one may output in `.jpg`, etc.\
Converts a raw pixel grid frame into an image.

**WARNING:** : If the image comes out glitched, you probably input the wrong  dimension.

## Make
**make** / **make default** / **make test** (DEFAULT)\
Generates `animate.o` and `tests/test`.

**make animate.o**
Generates `animate.o` only.

**make test_asan**\
Generates DEFAULT make with asan flag.

**make test_debug**\
Generates DEFAULT make with gdb flag.

**make doc**\
Generates reference manual pdf.

**make clean**\
Clean object files and doxygen files.

**make clobber**\
Return to initial repo state.

**WARNING:** Everytime you make, `animate.o` will take that as its last settings. For example, `./generate.sh` will rebuild `animate.o` with no flags, i.e. default build.

## WIP
- Layer drawing optimisation

## To be added
- Create non-filled circle
- Move layer to a specified layer number.
- Set user defined physics function.
- Option to warp edge.
- Other basic shapes