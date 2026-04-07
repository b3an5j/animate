// Quick hack
#include "../src/animate.c"
#include "../src/canvas_helper.c"
#include "../src/misc_helper.c"
#include "../src/physics_helper.c"
#include "../include/pixel_helper.h"
#include <stdio.h>
#include <stdlib.h>

#define BREAK printf("----------------------------------------\n")
#define TEST_PASS(msg) do { printf("[PASS] %s\n", msg); ++pass; ++total_tests; BREAK;} while(0)
#define TEST_FAIL(msg) do { printf("[FAIL] %s\n", msg); ++fail; ++total_tests; BREAK;} while(0)

#define BG_COLOR 0xFFFFFFFF
#define CIRCLE_COLOR 0xEE00FFFF
#define FORCED_CIRCLE_COLOR 0xFF00FFFF
#define RECT_COLOR 0xDEADBEEF
#define FORCED_RECT_COLOR 0xFFADBEEF
#define CANVAS_WIDTH 10
#define CANVAS_HEIGHT 10

int main(int argc, char* argv[])
{
    int total_tests = 0;
    int pass = 0;
    int fail = 0;

    /* TEST 1: Canvas Creation */
    struct canvas* canvas = animate_create_canvas(CANVAS_HEIGHT, CANVAS_WIDTH, BG_COLOR);
    if (!canvas) {
        TEST_FAIL("Canvas creation");
    }
    else {
        TEST_PASS("Canvas creation");
    }


    /* TEST 2: Rectangle Sprite */
    struct sprite* rect = animate_create_rectangle(5, 5, RECT_COLOR, true);
    if (!rect) {
        TEST_FAIL("Rectangle sprite creation");
    }
    else {
        TEST_PASS("Rectangle sprite creation");
    }


    /* TEST 3: Circle Sprite */
    struct sprite* circle = animate_create_circle(3, CIRCLE_COLOR, true);
    if (!circle) {
        TEST_FAIL("Circle sprite creation");
    }
    else {
        TEST_PASS("Circle sprite creation");
    }

    /* TEST 4: Foreign bmp Sprite */
    struct sprite* rick = animate_create_sprite("imgs/rick.bmp");
    if (!rick) {
        TEST_FAIL("Foreign sprite creation");
    }
    else {
        TEST_PASS("Foreign sprite creation");
    }

    /* TEST 4: Placement */
    struct sprite_placement* p1 = animate_place_sprite(canvas, circle, 0, 0);
    struct sprite_placement* p2 = animate_place_sprite(canvas, rect, 0, 0);
    if (!p1 || !p2) {
        TEST_FAIL("Native sprite placement");
        goto end_placement;
    }

    struct sprite_placement* p3 = animate_place_sprite(canvas, rick, 0, 0);
    if (!p3) {
        TEST_FAIL("Foreign sprite placement");
        goto end_placement;
    }
    TEST_PASS("Sprite placement");
end_placement:


    /* TEST 5: Animation params */
    animate_set_animation_params(p1, 5, 0, 0, 0);
    if (p1->params->x.velocity != 5 ||
        p1->params->y.velocity != 0 ||
        p1->params->x.acceleration != 0 ||
        p1->params->y.acceleration != 0) {
        TEST_FAIL("Animation params 1");
        goto end_params;
    }

    animate_set_animation_params(p2, 0, 5, 0, 0);
    if (p2->params->x.velocity != 0 ||
        p2->params->y.velocity != 5 ||
        p2->params->x.acceleration != 0 ||
        p2->params->y.acceleration != 0) {
        TEST_FAIL("Animation params 2");
        goto end_params;
    }

    animate_set_animation_params(p3, 5, 5, 0, 0);
    if (p3->params->x.velocity != 5 ||
        p3->params->y.velocity != 5 ||
        p3->params->x.acceleration != 0 ||
        p3->params->y.acceleration != 0) {
        TEST_FAIL("Animation params 2");
        goto end_params;
    }
    TEST_PASS("Animation params");
end_params:


    /* TEST 6: Placement removal */
    animate_destroy_placement(p3);
    p3 = NULL;
    if (circularlist_get_listsize(canvas->layers) != 2) {
        TEST_FAIL("Placement removal");
    }
    else {
        TEST_PASS("Placement removal");
    }

    /* TEST 6: Frame generation */
    size_t size = animate_frame_size_bytes(canvas);
    void* buffer = malloc(size);
    if (!buffer) {
        TEST_FAIL("Frame buffer alloc");
    }
    else {
        animate_generate_frame(canvas, 10, 10, buffer);
        TEST_PASS("Frame generation");
    }


    /* TEST 7: Layer movement */
    animate_placement_top(p1);
    animate_generate_frame(canvas, 0, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 3, 3, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_CIRCLE_COLOR) {
        TEST_FAIL("Layer movement 1");
        goto end_layer;
    }

    animate_placement_bottom(p1);
    animate_generate_frame(canvas, 0, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 3, 3, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Layer movement 2");
        goto end_layer;
    }

    animate_placement_up(p1);
    animate_generate_frame(canvas, 0, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 3, 3, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_CIRCLE_COLOR) {
        TEST_FAIL("Layer movement 3");
        goto end_layer;
    }
    animate_placement_down(p1);
    animate_generate_frame(canvas, 0, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 3, 3, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Layer movement 4");
        goto end_layer;
    }
    TEST_PASS("Layer movement");
end_layer:


    /* TEST 8: physics calculation*/
    // only velocity
    animate_generate_frame(canvas, 10, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 0, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != BG_COLOR) {
        TEST_FAIL("Physics calculation (v1)");
        goto end_physics;
    }
    if (pixel_get_color((color_t*)buffer, 0, 5, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Physics calculation (v2)");
        goto end_physics;
    }

    // only acceleration
    animate_set_animation_params(p2, 0, 0, 4, 0);
    animate_generate_frame(canvas, 10, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 0, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != BG_COLOR) {
        TEST_FAIL("Physics calculation (a1)");
        goto end_physics;
    }
    if (pixel_get_color((color_t*)buffer, 2, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Physics calculation (a2)");
        goto end_physics;
    }

    // both
    animate_set_animation_params(p2, 2, 0, 2, 0);
    animate_generate_frame(canvas, 10, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 0, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != BG_COLOR) {
        TEST_FAIL("Physics calculation (a1)");
        goto end_physics;
    }
    if (pixel_get_color((color_t*)buffer, 3, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Physics calculation (a2)");
        goto end_physics;
    }

    // both negative
    animate_set_animation_params(p2, 4, 0, -4, 0);
    animate_generate_frame(canvas, 10, 10, buffer);
    if (pixel_get_color((color_t*)buffer, 0, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != BG_COLOR) {
        TEST_FAIL("Physics calculation (a1)");
        goto end_physics;
    }
    if (pixel_get_color((color_t*)buffer, 2, 0, CANVAS_HEIGHT, CANVAS_WIDTH) != FORCED_RECT_COLOR) {
        TEST_FAIL("Physics calculation (a2)");
        goto end_physics;
    }
    TEST_PASS("Physics calculation");
end_physics:

    /* TEST 9: Cleanup */
    animate_destroy_sprite(circle); // should not destroy
    if (!circle) {
        TEST_FAIL("Destroyed sprite while in use");
        goto end_cleanup;
    }

    animate_destroy_placement(p1);
    animate_destroy_placement(p2);
    p1 = NULL;
    p2 = NULL;
    if (animate_destroy_sprite(rect)) {
        TEST_FAIL("Rectangle not freed (refcount issue)");
        goto end_cleanup;
    }
    if (animate_destroy_sprite(circle)) {
        TEST_FAIL("Circle not freed (refcount issue)");
        goto end_cleanup;
    }
    if (animate_destroy_sprite(rick)) {
        TEST_FAIL("Rick not freed (refcount issue)");
        goto end_cleanup;
    }

    animate_destroy_canvas(canvas);
    canvas = NULL;
    TEST_PASS("Cleanup");
end_cleanup:
    animate_destroy_placement(p1);
    animate_destroy_placement(p2);
    animate_destroy_canvas(canvas);

    free(buffer);

    if (pass == total_tests) {
        printf("ALL TESTS PASSED\n");
    }
    else {
        printf("PASSED %d / %d TESTS\n", pass, total_tests);
        printf("FAILED %d TESTS\n", fail);
    }
    return 0;
}