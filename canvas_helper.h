#ifndef CANVAS_HELPER_H
#define CANVAS_HELPER_H

#include "animate.h"
#include "misc_helper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

union channel;

struct skip_list;
struct circular_list;
struct list_node;

#define MAX_HEIGHT_SKIPLIST 32 // DO NOT CHANGE, SHOULD BE MORE THAN ENOUGH

struct skip_list* canvas_create_skiplist();
void canvas_destroy_skiplist(struct skip_list* skiplist);

struct circular_list* canvas_create_circularlist(struct skip_list* skiplist);
void canvas_destroy_circularlist(struct circular_list* circlist);

struct list_node* skiplist_search(struct skip_list* skiplist, uint32_t layer);
bool skiplist_insert(struct skip_list* skiplist,
    struct list_node* listnode, uint32_t layer);
bool skiplist_remove(struct skip_list* skiplist, uint32_t layer);

#endif