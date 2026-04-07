/**
 * @file canvas_helper.h
 * @brief Everything related to layering sprite placements
*/
#ifndef CANVAS_HELPER_H
#define CANVAS_HELPER_H

#include "animate.h"
#include "misc_helper.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Enum for moving sprite_placements
 */
typedef enum {
    TOP,        /**< Top layer */
    BOTTOM,     /**< Bottom layer */
    MOVEUP,     /**< Up a layer */
    MOVEDOWN    /**< Down a layer */
} PlacementMode;


/**
 * @struct circular_list
 * @brief Circular list for layering sprite placements.
 *
 * @details This implementation mimics the data structure stack.
 * Newly inserted placements go to the end of the list.
 * This means the head of the list is the bottom layer, while
 * tail of the list is the top layer in canvas.
 */
struct circular_list;

/**
 * @struct list_node
 * @brief Single node in the layering circular list.
 *
 * @details A node represents a layer of sprite placement.
 */
struct list_node;


/**
 * @brief Creates an empty circularlist, without nodes.
 *
 * @return The circular list
 */
struct circular_list* canvas_create_circularlist();

/**
 * @brief Destroy everything in a circular list, i.e. all placements.
 *
 * @param circlist The circular list to be destroyed
 */
void canvas_destroy_circularlist(struct circular_list* circlist);

/**
 * @brief Moves to the next layer in a circular list.
 *
 * @param circlist The circular list to be advanced
 * @return The sprite placement before advancing
 */
struct sprite_placement* canvas_advance_layer(struct circular_list* circlist);


/**
 * @brief Go to the first layer of the layering.
 *
 * @param circlist The circular list to be modified
 */
void circularlist_goto_first(struct circular_list* circlist);

/**
 * @brief Insert a placement as a new node in a circular list.
 *
 * @param circlist The circular list to be inserted
 * @param data Reference of sprite placement to be put
 * @param mode PlacementMode enum, should use TOP
 * @return The new node
 */
struct list_node* circularlist_insert(struct circular_list* circlist,
                                      struct sprite_placement* data,
                                      PlacementMode mode);

/**
 * @brief Move a layer in a circular list.
 *
 * @param circlist The circular list to be modified
 * @param listnode The node to be moved
 * @param mode PlacementMode enum
 * @return true when successful
 * @return false when failed
 */
bool circularlist_move(struct circular_list* circlist,
                       struct list_node* listnode,
                       PlacementMode mode);

/**
 * @brief Remove a layer in a circular list, without destroying placement.
 *
 * @param listnode The node to be removed
 * @param circlist The circular list to be modified
 * @return true when successful
 * @return false when failed
 */
bool circularlist_remove(struct list_node* listnode,
                         struct circular_list* circlist);


/**
 * @brief Get the number of layers of a circular list.
 *
 * @param circlist The circular list in question
 * @return Size of the circular list
 */
uint32_t circularlist_get_listsize(struct circular_list* circlist);

/**
 * @brief Get the circular list where a node sits.
 *
 * @param listnode The node in question
 * @return The circular list of the node
 */
struct circular_list* listnode_get_thislist(struct list_node* listnode);

#endif /* CANVAS_HELPER_H */