#include "canvas_helper.h"

static struct list_node* skiplist_create_head(struct list_node* prev_head);
static struct list_node* skiplist_create_nil(struct list_node* head, struct list_node* prev_nil);
static struct list_node* skiplist_create_node(
    struct list_node* after,
    struct list_node* before,
    struct list_node* above,
    struct list_node* below,
    uint32_t width,
    struct sprite_placement* placement
);
static void skiplist_print(struct skip_list* skiplist, uint32_t head, uint32_t tail);

/* Assumed ARGB32 Little Endian */
union channel {
    color_t raw;
    struct {
        uint8_t B;
        uint8_t G;
        uint8_t R;
        uint8_t A;
    };
};

struct skip_list {
    uint32_t size;
    struct list_node* head;
    struct list_node* tail;
    struct circular_list* circlist;
};

struct circular_list {
    uint32_t size;
    struct list_node* first;
    struct list_node* current;
    struct skip_list* skiplist;
};

struct list_node {
    struct list_node* after;
    struct list_node* before;
    struct list_node* above;
    struct list_node* below;
    uint32_t width;
    struct sprite_placement* placement;
};

struct skip_list* canvas_create_skiplist()
{
    /* INITIALISE SKIPLIST */
    struct skip_list* skiplist = malloc(sizeof(*skiplist));
    if (!skiplist) {
        DBG_PRINT("Failed to allocate memory for %s\n", "skip list");
        return NULL;
    }

    // careful
    skiplist->size = 0;
    skiplist->head = NULL;
    skiplist->tail = NULL;
    skiplist->circlist = NULL;

    /* MAKE SPECIAL NODES HEAD AND NIL */
    skiplist->head = skiplist_create_head(NULL);
    struct list_node* next_head = skiplist->head;
    struct list_node* next_nil = skiplist_create_nil(next_head, NULL);
    struct list_node* prev_head;
    for (uint8_t i = 1; i < MAX_HEIGHT_SKIPLIST; ++i) {
        prev_head = next_head;
        next_head = skiplist_create_head(prev_head);
        next_nil = skiplist_create_nil(next_head, prev_head->after);
        if (!next_head || !next_nil) {
            DBG_PRINT("Failed to allocate memory for %s\n",
                "skip list head/nil(s)");
            goto fail_skiplist;
        }
    }

    skiplist->tail = next_nil;
    return skiplist;

fail_skiplist:
    // flatten bottom
    if (next_head) {
        next_head->above->below = NULL;
        free(next_head);
    }
    canvas_destroy_skiplist(skiplist);
    return NULL;
}

void canvas_destroy_skiplist(struct skip_list* skiplist)
{
    if (!skiplist) {
        DBG_PRINT("Invalid argument %s\n", "non-existent skip list");
        return;
    }
    struct list_node* col = skiplist->head;
    struct list_node* row;
    while (col->below) {
        col = col->below;
    }

    struct list_node* right;
    struct list_node* up;
    while (col) { // col by col bottom up
        right = col->after;
        row = col;
        while (row) { // then whole 
            up = row->above;
            free(row);
            row = up;
        }
        col = right;
    }

    if (skiplist->circlist) {
        canvas_destroy_circularlist(skiplist->circlist);
    }
    free(skiplist);
}

static struct list_node* skiplist_create_node(
    struct list_node* after,
    struct list_node* before,
    struct list_node* above,
    struct list_node* below,
    uint32_t width,
    struct sprite_placement* placement
)
{
    struct list_node* listnode = malloc(sizeof(*listnode));
    if (!listnode) {
        DBG_PRINT("Failed to allocate memory for %s\n", "list node");
        return NULL;
    }

    listnode->after = after;
    listnode->before = before;
    listnode->above = above;
    listnode->below = below;
    listnode->width = width;
    listnode->placement = placement;
    return listnode;
}

static struct list_node* skiplist_create_head(struct list_node* prev_head)
{
    struct list_node* head = skiplist_create_node(
        NULL, NULL,
        prev_head, NULL,
        1, NULL);
    if (prev_head) {
        prev_head->below = head;
    }
    return head;
}

static struct list_node* skiplist_create_nil(struct list_node* head, struct list_node* prev_nil)
{
    struct list_node* nil = skiplist_create_node(
        NULL, head,
        prev_nil, NULL,
        0, NULL);
    head->after = nil;
    if (prev_nil) {
        prev_nil->below = nil;
    }
    return nil;
}

struct list_node* skiplist_search(struct skip_list* skiplist, uint32_t layer)
{
    if (!skiplist) {
        DBG_PRINT("Invalid argument %s\n", "non-existent skip list");
        return NULL;
    }

    struct list_node* cursor = skiplist->head;
    while (cursor->below) {
        cursor = cursor->below;
        while (cursor->width && cursor->width <= layer) {
            layer -= cursor->width;
            cursor = cursor->after;
        }
    }
    return cursor;
}

bool skiplist_insert(struct skip_list* skiplist, struct list_node* listnode, uint32_t layer)
{
    if (!skiplist) {
        DBG_PRINT("Invalid argument %s\n", "non-existent skip list");
        return false;
    }
    if (!listnode) {
        DBG_PRINT("Invalid argument %s\n", "non-existent list node");
        return false;
    }

    /* INITIALISE AT LEVEL 0 */
    struct list_node* successor = skiplist_search(skiplist, layer);
    struct list_node* predecessor = successor->before;
    if (!successor || !predecessor) {
        return false;
    }

    listnode->before = predecessor;
    listnode->after = successor;
    predecessor->after = listnode;
    successor->before = listnode;
    listnode->width = 1; // lvl 0 always 1

    /* BUILD TOWERS */
    uint32_t walked = 0;
    struct list_node* prev = listnode;
    struct list_node* pred = predecessor;
    for (uint8_t i = 0; i < MAX_HEIGHT_SKIPLIST; ++i) {
        // get the tower to the left
        while (pred && !pred->above) {
            walked += pred->width;
            pred = pred->before;
        }
        if (!pred) { break; }

        // climb to upper level
        pred = pred->above;


        // coin flip
        if (rand() % 2) { break; }

        uint32_t new_width = pred->width - walked; // right width        
        struct list_node* new = skiplist_create_node(
            pred->after,
            pred,
            NULL,
            prev,
            new_width,
            listnode->placement
        ); // build tower
        if (!new) { // forced short tower
            ++pred->width;
            break;
        }
        pred->width = walked + 1; // left width

        // stitch tower
        pred->after->before = new;
        pred->after = new;
        prev->above = new;

        // update level
        prev = new;
        walked = 0;
    }

    /* UPDATE UPPER LEVELS */
    while (pred) {
        while (pred->before && !pred->above) {
            pred = pred->before;
        }
        ++pred->width;
        pred = pred->above;
    }


    /* UPDATE SIZES */
    ++skiplist->size;
    if (skiplist->circlist) {
        ++skiplist->circlist->size;
    }
    return true;
}

bool skiplist_remove(struct skip_list* skiplist, uint32_t layer)
{
    if (!skiplist) {
        DBG_PRINT("Invalid argument %s\n", "non-existent skipl ist");
        return false;
    }
    if (!layer || layer > skiplist->size) {
        DBG_PRINT("Invalid argument %s %u\n", "layer", layer);
        return false;
    }

    struct list_node* bottom = skiplist_search(skiplist, layer);
    /* UPDATE WIDTHS */
    struct list_node* row = bottom;
    struct list_node* pred;
    // for bottom's tower
    while (row) {
        // get the tower to the left
        pred = row->before;
        while (pred && !pred->above) {
            pred = pred->before;
        }
        pred = pred->above;
        row = row->above;
        if (row) {
            pred->width += row->width - 1;
        }
        else {
            --pred->width;
        }
    }
    // go up closest left all the way
    struct list_node* left = pred->before;
    struct list_node* up = pred->above;
    while (up || left) {
        if (up) {
            pred = up;
        }
        else {
            while (left && !left->above) {
                left = left->before;
            }
            pred = left->above;
        }
        --pred->width;
        left = pred->before;
        up = pred->above;
    }

    /* DETACH LINKS */
    row = bottom;
    while (row) {
        row->before->after = row->after;
        row->after->before = row->before;
        bottom = row->above;
        free(row);
        row = bottom;
    }

    /* UPDATE SIZES */
    --skiplist->size;
    if (skiplist->circlist) {
        --skiplist->circlist->size;
    }
    return true;
}

static void skiplist_print(struct skip_list* skiplist, uint32_t start, uint32_t stop)
{
    struct list_node* row = skiplist->head;
    struct list_node* col;

    uint8_t counter = 0;
    while (row) {
        col = row;
        if (counter >= start && counter <= stop) {
            printf("[%d] NULL <-- ", counter);
            while (col) {
                printf("%p -%d-> ", col, col->width);
                col = col->after;
            }
            printf("NULL\n");
        }
        row = row->below;
        ++counter;
    }
}

struct circular_list* canvas_create_circularlist(struct skip_list* skiplist)
{
    if (!skiplist) {
        DBG_PRINT("Invalid argument %s\n", "non-existent skiplist");
        return NULL;
    }
    if (skiplist->circlist) {
        DBG_PRINT("%s\n", "Circular list already exists for this skiplist");
        return NULL;
    }

    struct circular_list* circlist = malloc(sizeof(*circlist));
    if (!circlist) {
        DBG_PRINT("Failed to allocate memory for %s\n", "circular list");
        return NULL;
    }

    circlist->size = skiplist->size;
    circlist->first = skiplist_search(skiplist, 1);
    circlist->current = circlist->first;
    circlist->skiplist = skiplist;
    return circlist;
}

void canvas_destroy_circularlist(struct circular_list* circlist)
{
    if (!circlist) { return; }
    circlist->skiplist->circlist = NULL;
    free(circlist);
}