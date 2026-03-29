#include "canvas_helper.h"

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
        DBG_PRINT(ERR_MALLOC, "circular list");
        DBG_PRINT(FAIL, "Create circular list");
        return NULL;
    }

    circlist->size = 0;
    circlist->first = NULL;
    circlist->last = NULL;
    circlist->current = NULL;
    DBG_PRINT(SUCCESS, "Create circular list");
    return circlist;
}

static struct list_node* create_listnode(struct circular_list* circlist,
    struct list_node* after, struct list_node* before,
    struct sprite_placement* data)
{
    struct list_node* listnode = malloc(sizeof(*listnode));
    if (!listnode) {
        DBG_PRINT(ERR_MALLOC, "list node");
        DBG_PRINT(FAIL, "Create list node");
        return NULL;
    }

    listnode->thislist = circlist;
    listnode->after = after;
    listnode->before = before;
    listnode->data = data;
    DBG_PRINT(SUCCESS, "Create list node");
    return listnode;
}

void canvas_destroy_circularlist(struct circular_list* circlist)
{
    if (!circlist) {
        DBG_PRINT(FREED, "Nothing / NULL");
        return;
    }

    struct list_node* curr = circlist->first;
    struct list_node* next;
    uint32_t size = circlist->size;
    for (uint32_t i = 0; i < size; ++i) {
        next = curr->after;
        animate_destroy_placement(curr->data);
        curr = next;
    }
    free(circlist);
    DBG_PRINT(FREED, "Circular list");
}

struct sprite_placement* canvas_advance_layer(struct circular_list* circlist)
{
    if (!circlist || !circlist->size) {
        DBG_PRINT(INVALID_ARG, "circlist");
        return NULL;
    }
    struct sprite_placement* ret = circlist->current->data;
    if (circlist->current->after) {
        circlist->current = circlist->current->after;
    }
    return ret;
}

void circularlist_goto_first(struct circular_list* circlist)
{
    circlist->current = circlist->first;
}

struct list_node* circularlist_insert(struct circular_list* circlist,
    struct sprite_placement* data, PlacementMode mode)
{
    if (!circlist) {
        DBG_PRINT(INVALID_ARG, "circlist");
        return NULL;
    }
    if (!data) {
        DBG_PRINT(INVALID_ARG, "data");
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
        circlist->first = new;
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
        circlist->last = new;
    }
    else {
        circlist->first = new;
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
        DBG_PRINT(INVALID_ARG, "circlist");
        return false;
    }
    if (!listnode || listnode->thislist != circlist) {
        DBG_PRINT(INVALID_ARG, "listnode");
        return false;
    }

    /* ONLY 1 NODE */
    if (circlist->size == 1) {
        return true;
    }
    /* ONLY 2 NODES */
    else if (circlist->size == 2) {
        if (mode == TOP && listnode == circlist->first) {
            struct list_node* temp = circlist->first;
            circlist->first = circlist->last;
            circlist->last = temp;
        }
        else if (mode == BOTTOM && listnode == circlist->last) {
            struct list_node* temp = circlist->first;
            circlist->first = circlist->last;
            circlist->last = temp;
        }
    }
    /* MULTIPLE NODES */
    else {
        // repair hole in advance
        if (listnode == circlist->first) {
            circlist->first = listnode->after;
        }
        if (listnode == circlist->last) {
            circlist->last = listnode->before;
        }

        listnode_unlink(listnode);
        listnode_insert_between(
            circlist->last,
            circlist->first,
            listnode
        );

        switch (mode) {
        case TOP:
            circlist->last = listnode;
            circlist->first = listnode->after;
            break;

        case BOTTOM:
            circlist->first = listnode;
            circlist->last = listnode->before;
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
        DBG_PRINT(INVALID_ARG, "circlist");
        return NULL;
    }
    if (!listnode || listnode->thislist != circlist) {
        DBG_PRINT(INVALID_ARG, "listnode");
        return NULL;
    }

    if (circlist->size == 1) {
        circlist->current = NULL;
        circlist->first = NULL;
        circlist->last = NULL;
    }
    else {
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
    listnode_unlink(listnode);

    free(listnode);
    DBG_PRINT(FREED, "List node");
    --circlist->size;
    DBG_PRINT(CUSTOM, "Removed list node from circular list.");
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