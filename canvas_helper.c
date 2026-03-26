#include "canvas_helper.h"

union channel {
    color_t raw;
    struct {
        uint8_t A;
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };
};

struct circular_list {
    uint32_t size;
    struct list_node* first;
    struct list_node* last; // for clarity
    struct list_node* current;
};

struct list_node {
    struct circular_list* thislist;
    struct list_node* after;
    struct list_node* before;
    struct sprite_placement* data;
};

struct circular_list* canvas_create_circularlist()
{
    struct circular_list* circlist = malloc(sizeof(*circlist));
    if (!circlist) {
        DBG_PRINT("Failed to allocate memory for %s\n" "circular list");
        return NULL;
    }

    circlist->size = 0;
    circlist->first = NULL;
    circlist->last = NULL;
    circlist->current = NULL;
    return circlist;
}

static struct list_node* create_listnode(struct circular_list* circlist,
    struct list_node* after, struct list_node* before,
    struct sprite_placement* data)
{
    struct list_node* listnode = malloc(sizeof(*listnode));
    if (!listnode) {
        DBG_PRINT("Failed to allocate memory for %s\n" "list node");
        return NULL;
    }

    listnode->thislist = circlist;
    listnode->after = after;
    listnode->before = before;
    listnode->data = data;
    return listnode;
}

void canvas_destroy_circularlist(struct circular_list* circlist)
{
    if (!circlist) {
        DBG_PRINT("%s is freed\n", "Nothing / NULL");
        return;
    }

    struct list_node* curr = circlist->first;
    for (uint32_t i = 0; i < circlist->size; ++i) {
        animate_destroy_placement(curr->data);
        free(curr);
        curr = curr->after;
    }
    free(circlist);
    DBG_PRINT("%s is freed\n", "Circular list");
}

struct sprite_placement* canvas_advance_layer(struct circular_list* circlist)
{
    if (!circlist || !circlist->size) {
        DBG_PRINT("Invalid argument %s\n", "circular list");
        return NULL;
    }
    struct sprite_placement* ret = circlist->current->data;
    if (circlist->current->after) {
        circlist->current = circlist->current->after;
    }
    return circlist->current->data;
}

void circularlist_goto_first(struct circular_list* circlist)
{
    circlist->current = circlist->first;
}

struct list_node* circularlist_insert(struct circular_list* circlist,
    struct sprite_placement* data, PlacementMode mode)
{
    if (!circlist) {
        DBG_PRINT("Invalid argument %s\n", "circular list");
        return NULL;
    }
    if (!data) {
        DBG_PRINT("Invalid argument %s\n", "data");
        return NULL;
    }
    if (mode != TOP && mode != BOTTOM) {
        return NULL;
    }

    struct list_node* new = create_listnode(circlist, NULL, NULL, data);
    if (!new) {
        return NULL;
    }
    new->thislist = circlist;

    // empty list
    if (!circlist->size) {
        circlist->last = new;
        circlist->current = new;
        new->after = new;
        new->before = new;
    }
    // non empty list
    else {
        new->after = circlist->first;
        new->before = circlist->last;

        circlist->first->before = new;
        circlist->last->after = new;
    }

    if (mode == TOP) {
        circlist->first = new;
    }
    else {
        circlist->last = new;
    }
    ++circlist->size;
    return new;
}

static void listnode_insert_between(struct list_node* prev, struct list_node* next,
    struct list_node* node)
{
    prev->after = node;
    next->before = node;
    node->after = next;
    node->before = prev;
}

static void listnode_unlink(struct list_node* node)
{
    node->before->after = node->after;
    node->after->before = node->before;
    node->before = NULL;
    node->after = NULL;
}

bool circularlist_move(struct circular_list* circlist,
    struct list_node* listnode, PlacementMode mode)
{
    if (!circlist) {
        DBG_PRINT("Invalid argument %s\n", "circular list");
        return false;
    }
    if (!listnode || listnode->thislist != circlist) {
        DBG_PRINT("Invalid argument %s\n", "list node");
        return false;
    }

    /* ONLY 1 NODE */
    if (circlist->size == 1) {
        return true;
    }
    /* ONLY 2 NODES */
    else if (circlist->size == 2) {
        if (mode == TOP && listnode == circlist->last) {
            struct list_node* temp = circlist->first;
            circlist->first = circlist->last;
            circlist->last = temp;
        }
        else if (mode == BOTTOM && listnode == circlist->first) {
            struct list_node* temp = circlist->first;
            circlist->first = circlist->last;
            circlist->last = temp;
        }
    }
    /* MULTIPLE NODES */
    else {
        listnode_unlink(listnode);
        switch (mode) {
        case TOP:
            listnode_insert_between(circlist->last, circlist->first, listnode);
            circlist->first = listnode;
            break;

        case BOTTOM:
            listnode_insert_between(circlist->last, circlist->first, listnode);
            circlist->last = listnode;
            break;

        default:
            break;
        }
    }
    return true;
}

bool circularlist_remove(struct list_node* listnode, struct circular_list* circlist)
{
    if (!circlist) {
        DBG_PRINT("Invalid argument %s\n", "circular list");
        return NULL;
    }
    if (!listnode || listnode->thislist != circlist) {
        DBG_PRINT("Invalid argument %s\n", "list node");
        return NULL;
    }

    if (circlist->size == 1) {
        circlist->current = NULL;
        circlist->first = NULL;
        circlist->last = NULL;
    }
    else {
        listnode_unlink(listnode);

        if (listnode == circlist->current) {
            circlist->current = listnode->after;
        }

        if (listnode == circlist->first) {
            circlist->first = listnode->after;
        }
        else if (listnode == circlist->last) {
            circlist->last = listnode->before;
        }
    }

    free(listnode);
    --circlist->size;
    return true;
}

uint32_t circularlist_get_listsize(struct circular_list* circlist)
{
    return circlist->size;
}

struct circular_list* listnode_get_thislist(struct list_node* listnode)
{
    return listnode->thislist;
}