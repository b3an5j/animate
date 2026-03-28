#ifndef CANVAS_HELPER_H
#define CANVAS_HELPER_H

#include "animate.h"
#include "misc_helper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    TOP,
    BOTTOM,
    MOVEUP,
    MOVEDOWN
} PlacementMode;

struct circular_list;
struct list_node;

struct circular_list* canvas_create_circularlist();
void canvas_destroy_circularlist(struct circular_list* circlist);
struct sprite_placement* canvas_advance_layer(struct circular_list* circlist);

void circularlist_goto_first(struct circular_list* circlist);
struct list_node* circularlist_insert(struct circular_list* circlist,
    struct sprite_placement* data, PlacementMode mode);
bool circularlist_move(struct circular_list* circlist,
    struct list_node* listnode, PlacementMode mode);
bool circularlist_remove(struct list_node* listnode, struct circular_list* circlist);

uint32_t circularlist_get_listsize(struct circular_list* circlist);
struct circular_list* listnode_get_thislist(struct list_node* listnode);

#endif